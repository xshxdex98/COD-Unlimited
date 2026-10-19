#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include <universal/q_shared.h>
#include "ui_mp.h"
#include <universal/q_parse.h>
#include <database/database.h>
#include <universal/com_files.h>
#include <stringed/stringed_hooks.h>
#include <win32/win_local.h>
#include <qcommon/unzip.h>
#include <string>
#include <vector>
#include <algorithm>

int ui_numArenas;
char *ui_arenaInfos[128];

static bool UI_ValidMapName(const char *name)
{
    if (!name || !*name || strlen(name) >= 60) return false;
    for (const char *p = name; *p; ++p)
        if (!isalnum((unsigned char)*p) && *p != '_' && *p != '-') return false;
    return true;
}

static std::string UI_FriendlyMapName(const char *name)
{
    if (!I_strnicmp(name, "mp_", 3)) name += 3;
    std::string display;
    bool wordStart = true;
    for (const unsigned char *p = (const unsigned char *)name; *p; ++p)
    {
        if (*p == '_' || *p == '-')
        {
            if (!display.empty() && display.back() != ' ') display += ' ';
            wordStart = true;
        }
        else
        {
            display += wordStart ? (char)toupper(*p) : (char)*p;
            wordStart = false;
        }
    }
    if (!display.empty() && display.back() == ' ') display.pop_back();
    return display;
}

static void UI_MergeArenaText(const char *text, const char *onlyMap = nullptr, bool existingOnly = false)
{
    char *parsed[128];
    const int count = UI_ParseInfos(text, 128, parsed);
    for (int i = 0; i < count; ++i)
    {
        char name[64];
        I_strncpyz(name, Info_ValueForKey(parsed[i], "map"), sizeof(name));
        if (!UI_ValidMapName(name) || (onlyMap && I_stricmp(name, onlyMap))) continue;
        int index = 0;
        for (; index < ui_numArenas; ++index)
            if (!I_stricmp(name, Info_ValueForKey(ui_arenaInfos[index], "map"))) break;
        if (existingOnly && index == ui_numArenas) continue;
        if (index < 128)
        {
            ui_arenaInfos[index] = parsed[i];
            if (index == ui_numArenas) ++ui_numArenas;
        }
    }
}

static void UI_LoadLooseArenaMetadata()
{
    char list[32768];
    int count = FS_GetFileList("mp", "arena", FS_LIST_ALL, list, sizeof(list));
    const char *name = list;
    for (int i = 0; i < count; ++i)
    {
        char path[256];
        Com_sprintf(path, sizeof(path), "mp/%s", name);
        int file = 0;
        int length = FS_FOpenFileByMode(path, &file, FS_READ);
        if (file)
        {
            if (length > 0 && length <= 8192)
            {
                char text[8193];
                int read = FS_Read((unsigned char *)text, length, file);
                if (read == length) { text[length] = 0; UI_MergeArenaText(text, nullptr, true); }
            }
            FS_FCloseFile(file);
        }
        name += strlen(name) + 1;
    }
}

static void UI_ReadArenaFile(const char *path, const char *map)
{
    FILE *file = fopen(path, "rb");
    if (!file) return;
    char text[8193];
    size_t size = fread(text, 1, sizeof(text) - 1, file);
    const bool complete = !ferror(file) && fgetc(file) == EOF;
    fclose(file);
    if (complete) { text[size] = 0; UI_MergeArenaText(text, map); }
}

// Orders arenas [first, ui_numArenas) by the name players see.
static void UI_SortArenasByDisplayName(int first)
{
    std::vector<std::pair<std::string, char *>> arenas;
    for (int i = first; i < ui_numArenas; ++i)
    {
        const char *longname = Info_ValueForKey(ui_arenaInfos[i], "longname");
        const char *display = SEH_StringEd_GetString(longname);
        arenas.emplace_back(display ? display : longname, ui_arenaInfos[i]);
    }
    std::stable_sort(arenas.begin(), arenas.end(), [](const auto &a, const auto &b) { return I_stricmp(a.first.c_str(), b.first.c_str()) < 0; });
    for (const auto &arena : arenas)
        ui_arenaInfos[first++] = arena.second;
}

// Adds the maps in <root>/<folder>/<map>/, with the arenas found loose or in their iwds.
static void UI_LoadMapFolder(const char *root, const char *folder)
{
    char pattern[512];
    Com_sprintf(pattern, sizeof(pattern), "%s/%s/*", root, folder);
    WIN32_FIND_DATAA entry;
    HANDLE search = FindFirstFileA(pattern, &entry);
    if (search == INVALID_HANDLE_VALUE) return;
    std::vector<std::string> names;
    do {
        if ((entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && UI_ValidMapName(entry.cFileName))
            names.emplace_back(entry.cFileName);
    } while (FindNextFileA(search, &entry));
    FindClose(search);
    for (const std::string &name : names)
    {
        char path[512], loaded[512];
        Com_sprintf(path, sizeof(path), "%s/%s/%s/%s.ff", root, folder, name.c_str(), name.c_str());
        // A map in more than one place is listed where the game will load it from.
        if (!FS_MapZonePath(name.c_str(), loaded, sizeof(loaded)) || I_stricmp(loaded, path)) continue;
        FILE *file = fopen(path, "rb");
        if (!file) continue;
        unsigned char header[12];
        const bool compatible = fread(header, 1, sizeof(header), file) == sizeof(header)
            && (!memcmp(header, "IWffu100", 8) || !memcmp(header, "IWffs100", 8))
            && header[8] == 5 && !header[9] && !header[10] && !header[11];
        fclose(file);
        if (!compatible) continue; // An IW4 .ff is not an installed IW3 map.
        int index = 0;
        for (; index < ui_numArenas; ++index)
            if (!I_stricmp(name.c_str(), Info_ValueForKey(ui_arenaInfos[index], "map"))) break;
        if (index == ui_numArenas && ui_numArenas < 128)
            UI_MergeArenaText(va("{ map \"%s\" longname \"%s\" }", name.c_str(), name.c_str()));
        Com_sprintf(path, sizeof(path), "%s/%s/%s/*", root, folder, name.c_str());
        search = FindFirstFileA(path, &entry);
        if (search == INVALID_HANDLE_VALUE) continue;
        do {
            if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            const char *extension = strrchr(entry.cFileName, '.');
            if (!extension) continue;
            Com_sprintf(path, sizeof(path), "%s/%s/%s/%s", root, folder, name.c_str(), entry.cFileName);
            if (!I_stricmp(extension, ".arena")) UI_ReadArenaFile(path, name.c_str());
            else if (!I_stricmp(extension, ".iwd"))
            {
                unzFile zip = unzOpen(path);
                if (!zip) continue;
                if (unzGoToFirstFile(zip) == UNZ_OK) do {
                    unz_file_info info;
                    char member[256];
                    if (unzGetCurrentFileInfo(zip, &info, member, sizeof(member), nullptr, 0, nullptr, 0) != UNZ_OK) break;
                    member[sizeof(member)-1] = 0;
                    const char *suffix = strrchr(member, '.');
                    if (suffix && !I_stricmp(suffix, ".arena") && info.uncompressed_size <= 8192 && unzOpenCurrentFile(zip) == UNZ_OK)
                    {
                        char text[8193];
                        int length = unzReadCurrentFile(zip, text, info.uncompressed_size);
                        int result = unzCloseCurrentFile(zip);
                        if (length == (int)info.uncompressed_size && result == UNZ_OK)
                        { text[length] = 0; UI_MergeArenaText(text, name.c_str()); }
                    }
                } while (unzGoToNextFile(zip) == UNZ_OK);
                unzClose(zip);
            }
        } while (FindNextFileA(search, &entry));
        FindClose(search);
    }
}

// Stock maps first, then COD Unlimited's, then the ones players installed, each group by name.
static void UI_LoadInstalledMapArenas()
{
    const char *roots[] = { fs_basepath->current.string, fs_homepath->current.string };
    const bool oneRoot = !I_stricmp(roots[0], roots[1]);
    for (const char *folder : FS_MAP_FOLDERS)
    {
        const int first = ui_numArenas;
        UI_LoadMapFolder(roots[0], folder);
        if (!oneRoot) UI_LoadMapFolder(roots[1], folder);
        UI_SortArenasByDisplayName(first);
    }
    Com_Printf(CON_CHANNEL_UI, "Map catalog: %i stock and installed maps\n", ui_numArenas);
}

int __cdecl UI_ParseInfos(const char *buf, int max, char **infos)
{
    uint8_t *v3; // eax
    char v5; // [esp+3h] [ebp-865h]
    char *v6; // [esp+8h] [ebp-860h]
    char *v7; // [esp+Ch] [ebp-85Ch]
    uint32_t v8; // [esp+10h] [ebp-858h]
    char info[1024]; // [esp+58h] [ebp-810h] BYREF
    char key[1028]; // [esp+458h] [ebp-410h] BYREF
    const char *token; // [esp+860h] [ebp-8h]
    int count; // [esp+864h] [ebp-4h]

    count = 0;
    while (1)
    {
        token = (const char *)Com_Parse(&buf);
        if (!*token)
            return count;
        if (strcmp(token, "{"))
        {
            Com_Printf(CON_CHANNEL_UI, "Missing { in info file\n");
            return count;
        }
        if (count == max)
        {
            Com_Printf(CON_CHANNEL_UI, "Max infos exceeded\n");
            return count;
        }
        info[0] = 0;
        while (1)
        {
            token = (const char *)Com_Parse(&buf);
            if (!*token)
                break;
            if (!strcmp(token, "}"))
                goto LABEL_14;
            I_strncpyz(key, (char *)token, 1024);
            token = (const char *)Com_ParseOnLine(&buf);
            if (!*token)
                token = "<NULL>";
            Info_SetValueForKey(info, key, token);
        }
        Com_Printf(CON_CHANNEL_UI, "Unexpected end of info file\n");
    LABEL_14:
        v8 = strlen(va("%d", 64));
        v3 = UI_Alloc(strlen(info) + v8 + 6, 1);
        infos[count] = (char *)v3;
        if (infos[count])
        {
            v7 = info;
            v6 = infos[count];
            do
            {
                v5 = *v7;
                *v6++ = *v7++;
            } while (v5);
            ++count;
        }
    }
}

void __cdecl UI_LoadArenas()
{
    sharedUiInfo.mapCount = 0;
    UI_LoadArenasFromFile();
    for (int n = 0; n < ui_numArenas && sharedUiInfo.mapCount < 128; ++n)
    {
        mapInfo &map = sharedUiInfo.mapList[sharedUiInfo.mapCount];
        map = {};
        map.mapLoadName = String_Alloc(Info_ValueForKey(ui_arenaInfos[n], "map"));
        const char *display = Info_ValueForKey(ui_arenaInfos[n], "longname");
        const std::string friendly = UI_FriendlyMapName(map.mapLoadName);
        map.mapName = String_Alloc(*display && I_stricmp(display, map.mapLoadName) ? display : friendly.c_str());
        map.imageName = String_Alloc(va("loadscreen_%s", map.mapLoadName));
        char path[512];
        if (!FS_MapZonePath(map.mapLoadName, path, sizeof(path)))
            map.levelShot = Material_RegisterHandle(map.imageName, IMAGE_TRACK_UI);
        const char *modes = Info_ValueForKey(ui_arenaInfos[n], "gametype");
        map.typeBits = *modes ? 0 : -1;
        if (*modes)
        {
            Com_BeginParseSession("map arena gametypes");
            const char *cursor = modes;
            for (;;)
            {
                const char *mode = (const char *)Com_Parse(&cursor);
                if (!mode || !*mode) break;
                for (int i = 0; i < sharedUiInfo.numGameTypes && i < 32; ++i)
                    if (!I_stricmp(mode, sharedUiInfo.gameTypes[i].gameType)) map.typeBits |= (1u << i);
            }
            Com_EndParseSession();
        }
        ++sharedUiInfo.mapCount;
    }
}

const char *UI_LoadArenasFromFile_LoadObj()
{
    char *result; // eax
    const char *v1; // [esp+14h] [ebp-24A4h]
    char string[132]; // [esp+18h] [ebp-24A0h] BYREF
    char *v3; // [esp+9Ch] [ebp-241Ch]
    char listbuf[1024]; // [esp+A0h] [ebp-2418h] BYREF
    char buffer[8196]; // [esp+4A0h] [ebp-2018h] BYREF
    int len; // [esp+24A8h] [ebp-10h]
    int f; // [esp+24ACh] [ebp-Ch] BYREF
    int v8; // [esp+24B0h] [ebp-8h]
    uint32_t v9; // [esp+24B4h] [ebp-4h]

    ui_numArenas = 0;
    result = (char *)FS_GetFileList("mp", "arena", FS_LIST_PURE_ONLY, listbuf, 1024);
    v1 = result;
    v3 = listbuf;
    v8 = 0;
    while (v8 < (int)v1)
    {
        v9 = strlen(v3);
        snprintf(string, ARRAYSIZE(string), "%s/%s", "mp", v3);
        len = FS_FOpenFileByMode(string, &f, FS_READ);
        if (f)
        {
            if (len < 0x2000)
            {
                FS_Read((uint8_t *)buffer, len, f);
                buffer[len] = 0;
                FS_FCloseFile(f);
                ui_numArenas += UI_ParseInfos(buffer, 64 - ui_numArenas, &ui_arenaInfos[ui_numArenas]);
            }
            else
            {
                Com_PrintError(CON_CHANNEL_UI, "file too large: %s is %i, max allowed is %i", string, len, 0x2000);
                FS_FCloseFile(f);
            }
        }
        else
        {
            Com_PrintError(CON_CHANNEL_UI, "file not found: %s\n", string);
        }
        ++v8;
        result = &v3[v9 + 1];
        v3 = result;
    }
    return result;
}

void UI_LoadArenasFromFile()
{
    ui_numArenas = 0;
    if (IsFastFileLoad())
        UI_LoadArenasFromFile_FastFile();
    else
        UI_LoadArenasFromFile_LoadObj();
    UI_LoadInstalledMapArenas();
    if (IsFastFileLoad()) UI_LoadLooseArenaMetadata();
}

void UI_LoadArenasFromFile_FastFile()
{
    RawFile *rawfile; // [esp+8h] [ebp-8h]

    rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, "mp/cod2maps.arena").rawfile;
    if (rawfile)
        ui_numArenas = UI_ParseInfos(rawfile->buffer, 64 - ui_numArenas, &ui_arenaInfos[ui_numArenas]);
    else
        Com_PrintError(CON_CHANNEL_UI, "file not found: %s\n", "mp/cod2maps.arena");
}

