//! Read-only IW4 extraction bridge. Output is intermediate data, not an IW3 fastfile.
use fastfile_iw4::{AssetSink, AssetType, Ptr, ScriptStrings, ZoneStream};
use std::{
    fs,
    io::{BufWriter, Write},
    path::PathBuf,
};

struct Sink {
    assets: usize,
}
impl AssetSink for Sink {
    fn set_script_strings(&mut self, _: ScriptStrings) {}
    fn load_asset(
        &mut self,
        stream: &mut ZoneStream<'_>,
        _: usize,
        kind: AssetType,
        slot: Ptr,
    ) -> fastfile_iw4::Result<()> {
        fastfile_iw4::load_asset_at(stream, kind, slot)?;
        self.assets += 1;
        Ok(())
    }
}

fn main() {
    let worker = std::thread::Builder::new()
        .name("iw4-extract".into())
        .stack_size(32 * 1024 * 1024)
        .spawn(|| run().map_err(|e| e.to_string()))
        .expect("start extraction worker");
    if let Err(error) = worker.join().expect("extraction worker panicked") {
        eprintln!("{error}");
        std::process::exit(1);
    }
}

fn run() -> Result<(), Box<dyn std::error::Error>> {
    let mut args = std::env::args_os().skip(1);
    let input = PathBuf::from(args.next().ok_or("Expected MW2 .ff input")?);
    let output = PathBuf::from(args.next().ok_or("Expected output directory")?);
    let image = asset_transport::open_zone(&input)?;
    let header = image.header()?;
    let mut memory = asset_transport::ZoneMemory::for_header(&header);
    let mut stream = memory
        .stream(&image.bytes)
        .map_err(|e| format!("stream: {e:?}"))?;
    let mut sink = Sink { assets: 0 };
    fastfile_iw4::load_zone(&mut stream, &mut sink).map_err(|e| format!("asset walk: {e:?}"))?;
    let world = stream.gfx_world().ok_or("Zone has no GfxWorld")?;
    let vertices = world.vertices.ok_or("World has no vertices")?;
    let indices = world.indices.ok_or("World has no indices")?;
    let surfaces = world.surfaces.ok_or("World has no surfaces")?;
    fs::create_dir_all(&output)?;
    let mut obj = BufWriter::new(fs::File::create(output.join("world.obj"))?);
    writeln!(
        obj,
        "# COD Unlimited IW4 intermediate; original CoD coordinates, inches, Z up"
    )?;
    for i in 0..world.vertex_count {
        let v = vertices.at(i * 44);
        let xyz = [
            stream.f32_at(v, 0),
            stream.f32_at(v, 4),
            stream.f32_at(v, 8),
        ]
        .map(|v| v.map_err(|e| format!("vertex: {e:?}")));
        let [x, y, z] = xyz;
        writeln!(obj, "v {} {} {}", x?, y?, z?)?;
    }
    let mut triangles = 0usize;
    for i in 0..world.surface_count {
        let surface = surfaces.at(i * stream.layout(24, 32));
        let read_error = |e| format!("surface {i}: {e:?}");
        let base = stream.u32_at(surface, 4).map_err(read_error)? as usize;
        let count = stream.u16_at(surface, 8).map_err(read_error)? as usize;
        let tris = stream.u16_at(surface, 10).map_err(read_error)? as usize;
        let first = stream.u32_at(surface, 12).map_err(read_error)? as usize;
        if base + count > world.vertex_count || first + tris * 3 > world.index_count {
            return Err(format!("Invalid index range in surface {i}").into());
        }
        writeln!(obj, "g surface_{i}")?;
        for t in 0..tris {
            let mut face = [0; 3];
            for (corner, index) in face.iter_mut().enumerate() {
                let local = stream
                    .u16_at(indices, (first + t * 3 + corner) * 2)
                    .map_err(read_error)? as usize;
                if local >= count {
                    return Err(format!("Invalid vertex in surface {i}").into());
                }
                *index = base + local + 1;
            }
            writeln!(obj, "f {} {} {}", face[0], face[1], face[2])?;
            triangles += 1;
        }
    }
    obj.flush()?;
    if let Some(entities) = stream.map_ents() {
        if let Some(pointer) = entities.entity_string {
            let bytes = stream
                .slice_at(pointer, 0, entities.entity_chars)
                .map_err(|e| format!("map entities: {e:?}"))?;
            fs::write(
                output.join("mw2_entities.ents"),
                bytes.strip_suffix(&[0]).unwrap_or(bytes),
            )?;
        }
    }
    if let Some(collision) = stream.clip_map() {
        let mut obj = BufWriter::new(fs::File::create(output.join("collision_triangles.obj"))?);
        if let (Some(vertices), Some(indices)) = (collision.verts, collision.tri_indices) {
            for i in 0..collision.vert_count {
                let v = vertices.at(i * 12);
                writeln!(
                    obj,
                    "v {} {} {}",
                    stream
                        .f32_at(v, 0)
                        .map_err(|e| format!("collision: {e:?}"))?,
                    stream
                        .f32_at(v, 4)
                        .map_err(|e| format!("collision: {e:?}"))?,
                    stream
                        .f32_at(v, 8)
                        .map_err(|e| format!("collision: {e:?}"))?
                )?;
            }
            for i in 0..collision.tri_count {
                let mut face = [0; 3];
                for (corner, index) in face.iter_mut().enumerate() {
                    *index = stream
                        .u16_at(indices, (i * 3 + corner) * 2)
                        .map_err(|e| format!("collision index: {e:?}"))?
                        as usize
                        + 1;
                    if *index > collision.vert_count {
                        return Err("Invalid collision vertex".into());
                    }
                }
                writeln!(obj, "f {} {} {}", face[0], face[1], face[2])?;
            }
        }
        obj.flush()?;
        fs::write(
            output.join("collision_summary.txt"),
            format!(
                "Planes: {}\nBrushes: {}\nCollision vertices: {}\nCollision triangles: {}\nStatic model colliders: {}\nBrush hull conversion pending; triangle OBJ is not complete collision.\n",
                collision.plane_count,
                collision.brush_count,
                collision.vert_count,
                collision.tri_count,
                collision.static_model_count
            ),
        )?;
    }
    let report = format!(
        "Input: {}\nWire format: {:?}\nAssets walked: {}\nWorld vertices: {}\nWorld surfaces: {}\nWorld triangles: {}\nStatic models: {}\nCollision present: {}\nMap entities present: {}\nStatus: intermediate geometry only; IW3 conversion pending\n",
        input.display(),
        image.iw4_wire_format,
        sink.assets,
        world.vertex_count,
        world.surface_count,
        triangles,
        world.smodel_count,
        stream.clip_map().is_some(),
        stream.map_ents().is_some()
    );
    fs::write(output.join("extraction.txt"), &report)?;
    print!("{report}");
    Ok(())
}
