#pragma once

// Included by ui_main_mp.cpp after its UI context declarations.
static std::string UI_JsonString(const char *value)
{
    std::string out = "\"";
    if (value)
        for (const char *p = value; *p; ++p)
        {
            if (*p == '"' || *p == '\\') out += '\\';
            if (*p == '\n') out += "\\n";
            else if (*p == '\r') out += "\\r";
            else if (*p == '\t') out += "\\t";
            else if ((unsigned char)*p >= 32) out += *p;
        }
    return out + "\"";
}

static void UI_DumpFrontendInventory(UiContext *dc)
{
    if (!Dvar_GetBool("ui_dumpFrontend")) return;
    std::string out = "[";
    for (int m = 0; m < dc->menuCount; ++m)
    {
        menuDef_t *menu = dc->Menus[m];
        if (m) out += ',';
        out += "{\"name\":" + UI_JsonString(menu->window.name)
            + ",\"open\":" + UI_JsonString(menu->onOpen)
            + ",\"close\":" + UI_JsonString(menu->onClose)
            + ",\"esc\":" + UI_JsonString(menu->onESC) + ",\"items\":[";
        for (int i = 0; i < menu->itemCount; ++i)
        {
            itemDef_s *item = menu->items[i];
            if (i) out += ',';
            out += "{\"name\":" + UI_JsonString(item->window.name)
                + ",\"text\":" + UI_JsonString(item->text)
                + ",\"dvar\":" + UI_JsonString(item->dvar)
                + ",\"action\":" + UI_JsonString(item->action)
                + ",\"focus\":" + UI_JsonString(item->onFocus)
                + ",\"material\":" + UI_JsonString(item->window.background ? item->window.background->info.name : nullptr);
            const rectDef_s &r = item->window.rectClient;
            out += va(",\"type\":%i,\"dataType\":%i,\"owner\":%i,\"flags\":%i,\"dynamic\":%i,\"feeder\":%g,\"rect\":[%g,%g,%g,%g,%i,%i]}",
                item->type, item->dataType, item->window.ownerDraw, item->window.staticFlags, item->window.dynamicFlags[0], item->special,
                r.x, r.y, r.w, r.h, r.horzAlign, r.vertAlign);
        }
        out += "]}";
    }
    out += "]";
    FS_WriteFile((char *)"frontend_menu_inventory.json", (char *)out.c_str(), out.size());
    Dvar_SetBoolByName("ui_dumpFrontend", false);
}


// The runtime script tokenizer needs semicolons separated from arguments.
// Loose .menu scripts receive this spacing from PC_Script_Parse automatically.
static const char *CU_Script(const char *source)
{
    if (!source) return nullptr;
    std::string script;
    for(const char *p=source;*p;++p)
    {
        if (*p==';') script+=" ; "; else script+=*p;
    }
    return String_Alloc(script.c_str());
}

static bool s_cuFrontendThemed;
static void CU_Color(float *dst, float r, float g, float b, float a = 1.0f)
{ dst[0] = r; dst[1] = g; dst[2] = b; dst[3] = a; }

static void CU_Rect(itemDef_s *item, float x, float y, float w, float h)
{
    item->window.rectClient = { x, y, w, h, 4, 4 };
    item->window.rect = item->window.rectClient;
    item->rectXExp = {}; item->rectYExp = {}; item->rectWExp = {}; item->rectHExp = {};
    item->textRect[0] = {};
}

static itemDef_s *CU_Item(menuDef_t *menu, const char *name, float x, float y, float w, float h)
{
    iassert(menu->itemCount < 256);
    auto *item = (itemDef_s *)UI_Alloc(sizeof(itemDef_s), 4);
    memset(item, 0, sizeof(*item));
    item->parent = menu;
    item->window.name = String_Alloc(name);
    item->window.dynamicFlags[0] = 4 | 0x10000;
    item->window.staticFlags = 0x100000;
    item->fontEnum = 0; item->textscale = .23f; item->textaligny = 16;
    item->imageTrack = 3;
    CU_Color(item->window.foreColor, .85f, .89f, .86f);
    CU_Rect(item, x, y, w, h);
    menu->items[menu->itemCount++] = item;
    return item;
}

static itemDef_s *CU_Panel(menuDef_t *menu, const char *name, float x, float y, float w, float h, float alpha = .85f)
{
    auto *item = CU_Item(menu, name, x, y, w, h);
    item->window.style = 1;
    CU_Color(item->window.backColor, .018f, .028f, .024f, alpha);
    return item;
}

static itemDef_s *CU_Label(menuDef_t *menu, const char *name, const char *text, float x, float y, float w, float scale = .23f)
{
    auto *item = CU_Item(menu, name, x, y, w, 22);
    item->text = String_Alloc(text); item->textscale = scale;
    return item;
}

static itemDef_s *CU_Button(menuDef_t *menu, const char *name, const char *text, const char *action,
    float x, float y, float w, bool primary = false, bool withIcon = true)
{
    auto *item = CU_Label(menu, name, text, x, y, w, .27f);
    item->type = item->dataType = 1; item->window.staticFlags = 0; item->fontEnum = 2;
    item->window.style = 1; item->window.border = 1; item->window.borderSize = .5f;
    item->window.rectClient.h = 27; item->window.rect.h = 27;
    item->textalignx = withIcon ? 29.0f : 6.0f; item->textaligny = 18;
    CU_Color(item->window.backColor, primary ? .13f : .035f, primary ? .22f : .052f, primary ? .115f : .047f, primary ? .83f : .72f);
    CU_Color(item->window.borderColor, primary ? .40f : .20f, primary ? .57f : .25f, primary ? .33f : .23f, .7f);
    item->action = CU_Script(action);
    if (withIcon)
    {
        auto *edge = CU_Panel(menu, va("%s_edge",name), x, y, 1.6f, 27);
        CU_Color(edge->window.backColor, .64f, .9f, .49f, primary ? 1.0f : 0.0f);
        if (strstr(text,"BACK"))
            CU_Label(menu,va("%s_icon",name),"<",x+9,y+2,15,.27f)->fontEnum=2;
        else
        {
            const char *iconName="codunlimited_icon_settings";
            if (strstr(text,"SESSION") || strstr(text,"LOCAL")) iconName="codunlimited_icon_local";
            if (strstr(text,"SERVER") || strstr(text,"INTERNET") || !strcmp(text,"LAN") || strstr(text,"FAVORITE")) iconName="codunlimited_icon_multiplayer";
            if (strstr(text,"MOD") && !strstr(text,"MODE")) iconName="codunlimited_icon_mods";
            if (strstr(text,"CHALLENGE") || strstr(text,"PROGRESS") || strstr(name,"tier_")) iconName="codunlimited_icon_extras";
            if (strstr(text,"VIDEO") || strstr(text,"DISPLAY") || strstr(text,"TEXTURE")) iconName="codunlimited_icon_video";
            if (strstr(text,"AUDIO")) iconName="codunlimited_icon_audio";
            if (strstr(text,"CONTROLS") || strstr(text,"MOUSE") || strstr(text,"MOVEMENT") || strstr(text,"COMBAT") || strstr(text,"INTERACT")) iconName="codunlimited_icon_controls";
            if (strstr(text,"NETWORK") || strstr(text,"REFRESH")) iconName="codunlimited_icon_network";
            auto *icon=CU_Item(menu,va("%s_icon",name),x+9,y+7,12,12);
            icon->window.style=3; icon->window.background=Material_RegisterHandle(iconName,3);
            CU_Color(icon->window.foreColor,primary?.72f:.72f,primary?.94f:.77f,primary?.59f:.74f);
        }
    }
    item->onFocus = CU_Script(va("setitemcolor %s backcolor .13 .22 .115 .83; setitemcolor %s bordercolor .40 .57 .33 .85; setitemcolor %s_edge backcolor .64 .9 .49 1; setitemcolor %s_icon forecolor .72 .94 .59 1; play mouse_over;",name,name,name,name));
    item->leaveFocus = CU_Script(va("setitemcolor %s backcolor %s; setitemcolor %s bordercolor %s; setitemcolor %s_edge backcolor .64 .9 .49 %i; setitemcolor %s_icon forecolor %s;",name,primary?".13 .22 .115 .83":".035 .052 .047 .72",name,primary?".40 .57 .33 .85":".20 .25 .23 .7",name,primary?1:0,name,primary?".72 .94 .59 1":".72 .77 .74 1"));
    return item;
}

static menuDef_t *CU_Menu(UiContext *dc, menuDef_t *stock, const char *name, const char *title, const char *subtitle)
{
    auto *menu = (menuDef_t *)UI_Alloc(sizeof(menuDef_t), 4);
    *menu = *stock;
    menu->window.name = String_Alloc(name);
    menu->window.rect = {0, 0, 640, 480, 4, 4};
    menu->window.rectClient = menu->window.rect;
    menu->window.style = 0; menu->window.border = 0; menu->window.background = nullptr;
    menu->window.ownerDraw = 0; menu->window.dynamicFlags[0] = 0;
    menu->fullScreen = 1; menu->blurRadius = 0;
    menu->rectXExp = {}; menu->rectYExp = {}; menu->visibleExp = {};
    menu->items = (itemDef_s **)UI_Alloc(256 * sizeof(itemDef_s *), 4); menu->itemCount = 0;
    menu->cursorItem[0] = -1;
    CU_Color(menu->focusColor, .72f, .94f, .58f);
    CU_Color(menu->disableColor, .40f, .45f, .42f);
    auto *bg = CU_Item(menu, "cu_background", 0, 0, 640, 480);
    bg->window.style = 3; bg->window.background = Material_RegisterHandle("codunlimited_frontend", 3);
    CU_Color(bg->window.foreColor, .45f, .49f, .45f);
    CU_Panel(menu, "cu_shade", 0, 0, 640, 480, .68f);
    CU_Label(menu, "cu_brand", "COD UNLIMITED  /  MULTIPLAYER", 25, 12, 500, .17f);
    auto *heading = CU_Label(menu, "cu_heading", title, 25, 36, 580, .43f);
    heading->fontEnum = 2; heading->textaligny = 22;
    CU_Label(menu, "cu_subtitle", subtitle, 26, 65, 590, .18f);
    CU_Panel(menu, "cu_footer", 0, 458, 640, 22, .9f);
    CU_Label(menu, "cu_version", "COD UNLIMITED   |   v0.0.1", 25, 460, 400, .16f);
    return menu;
}

static itemDef_s *CU_Control(menuDef_t *menu, const itemDef_s *source, const char *name,
    float x, float y, float w, float h = 24)
{
    auto *item = CU_Item(menu, name, x, y, w, h);
    *item = *source;
    item->parent = menu; item->window.name = String_Alloc(name);
    item->window.background = nullptr; item->window.style = 1; item->window.border = 0;
    item->text = (item->type == 4 || item->type == 9 || item->type == 10 || item->type == 14) ? " " : nullptr; item->textExp = {}; item->materialExp = {};
    item->textalignx = 6; item->textaligny = 16; item->textscale = .23f;
    item->fontEnum = 0; item->textAlignMode = 0; item->textStyle = 0;
    if (item->type == 10) item->textaligny = 3;
    item->onFocus = CU_Script("play mouse_over;"); item->leaveFocus = nullptr;
    CU_Color(item->window.foreColor, .84f, .91f, .81f);
    CU_Color(item->window.backColor, .035f, .052f, .045f, .85f);
    CU_Color(item->window.borderColor, .24f, .34f, .26f);
    CU_Rect(item, x, y, w, h);
    if (item->type == 6 && source->typeData.listBox)
    {
        auto *list = (listBoxDef_s *)UI_Alloc(sizeof(listBoxDef_s), 4);
        *list = *source->typeData.listBox;
        list->elementHeight = 20; list->elementWidth = w; item->textaligny = -4;
        CU_Color(list->selectBorder, .38f, .65f, .32f, .85f);
        if (list->numColumns == 1) list->columnInfo[0].width = w - 22;
        item->typeData.listBox = list;
    }
    else if ((item->type == 4 || item->type == 9 || item->type == 10) && source->typeData.editField)
    {
        auto *edit = (editFieldDef_s *)UI_Alloc(sizeof(editFieldDef_s), 4);
        *edit = *source->typeData.editField;
        edit->maxPaintChars = 23; item->typeData.editField = edit;
    }
    return item;
}

static void CU_Replace(UiContext *dc, menuDef_t *menu)
{
    Menu_UpdatePosition(0, menu);
    for (int i = 0; i < dc->menuCount; ++i)
        if (!I_stricmp(dc->Menus[i]->window.name, menu->window.name))
        { dc->Menus[i] = menu; return; }
    iassert(dc->menuCount < 640);
    dc->Menus[dc->menuCount++] = menu;
}

static void CU_LocalPlay(UiContext *dc)
{
    menuDef_t *stock = Menus_FindByName(dc, "createserver");
    if (!stock || stock->itemCount <= 91) return;
    for (int page = 0; page < 2; ++page)
    {
        const char *name = page ? "cu_local_advanced" : "createserver";
        auto *menu = CU_Menu(dc, stock, name, "LOCAL PLAY", "Set up your session, choose a map and deploy.");
        menu->onESC = CU_Script("close self; open main;");
        menu->onClose = nullptr;
        CU_Button(menu, "cu_basic", "SESSION", "close self; open createserver;", 26, 94, 110, !page);
        CU_Button(menu, "cu_advanced", "ADVANCED", "close self; open cu_local_advanced;", 142, 94, 110, page);
        CU_Panel(menu, "cu_settings_panel", 26, 132, 333, 272);
        const int basic[] = {20,26,38,56,63,69};
        const int advanced[] = {32,44,50,75,81};
        const char *basicLabels[] = {"Game mode", "Server name", "Maximum players", "Password", "Voice chat", "Auto-balance teams"};
        const char *advancedLabels[] = {"Dedicated server", "Minimum ping", "Maximum ping", "Allow voting", "PunkBuster"};
        int count = page ? 5 : 6;
        for (int row = 0; row < count; ++row)
        {
            int index = page ? advanced[row] : basic[row];
            float y = 145.0f + 34.0f * row;
            CU_Label(menu, va("cu_label_%i", row), page ? advancedLabels[row] : basicLabels[row], 38, y, 150, .22f);
            CU_Control(menu, stock->items[index], va("cu_control_%i", row), 191, y, 154);
        }
        auto *rules = CU_Button(menu, "cu_rules", "GAME MODE RULES", stock->items[87]->action, 38, 365, 208);
        rules->visibleExp = stock->items[87]->visibleExp;
        CU_Label(menu, "cu_map_heading", "MAP SELECTION", 382, 99, 230, .23f);
        CU_Panel(menu, "cu_map_panel", 378, 132, 236, 272);
        CU_Control(menu, stock->items[89], "mappreview", 384, 138, 224, 131);
        CU_Control(menu, stock->items[88], "maplist", 384, 279, 224, 118);
        CU_Button(menu, "cu_back", "BACK", "close self; open main;", 26, 420, 90);
        CU_Button(menu, "accept", "START SESSION", stock->items[91]->action, 474, 420, 140, true);
        CU_Replace(dc, menu);
    }
}


static const char *CU_SourceLabel(menuDef_t *stock, int index)
{
    const char *dvar = stock->items[index]->dvar;
    struct Label { const char *dvar; const char *text; };
    static const Label labels[] = {
        {"ui_r_mode", "Resolution"},
        {"ui_r_displayRefresh", "Refresh rate"},
        {"ui_r_aspectratio", "Aspect ratio"},
        {"ui_r_aasamples", "Anti-aliasing"},
        {"r_gamma", "Brightness"},
        {"ui_r_vsync", "V-sync"},
        {"r_multiGpu", "Dual video cards"},
        {"sm_enable", "Shadows"},
        {"r_specular", "Specular maps"},
        {"r_dof_enable", "Depth of field"},
        {"r_glow_allowed", "Glow"},
        {"r_dlightlimit", "Dynamic lights"},
        {"ui_zfeather", "Soft smoke edges"},
        {"ragdoll_enable", "Ragdolls"},
        {"fx_marks", "Bullet impacts"},
        {"r_lodScaleRigid", "Model detail"},
        {"r_drawWater", "Water detail"},
        {"ai_corpsecount", "Corpses"},
        {"r_texFilterMipMode", "Texture filtering"},
        {"r_texFilterAnisoMin", "Anisotropic filtering"},
        {"ui_r_picmip_manual", "Texture quality"},
        {"ui_r_picmip", "Texture resolution"},
        {"ui_r_picmip_bump", "Normal map resolution"},
        {"ui_r_picmip_spec", "Specular resolution"},
        {"snd_volume", "Master volume"},
        {"ui_snd_enableEq", "EQ filter"},
        {"ui_snd_khz", "Sound quality"},
        {"ui_outputConfig", "Speaker configuration"},
        {"cl_voice", "Voice chat"},
        {"winvoice_mic_reclevel", "Recording level"},
        {"cg_blood", "Show blood"},
        {"monkeytoy", "Enable console"},
        {"cg_subtitles", "Subtitles"},
        {"cg_drawCrosshair", "Crosshair"},
        {"ui_language", "Language"},
        {"cl_punkbuster", "PunkBuster"},
        {"cl_allowdownload", "Allow downloads"},
        {"ui_name", "Player name"},
        {"ui_mousePitch", "Invert mouse"},
        {"+leanleft", "Lean left"},
        {"+leanright", "Lean right"},
        {"+lookup", "Look up"},
        {"+lookdown", "Look down"},
        {"+left", "Turn left"},
        {"+right", "Turn right"},
        {"+mlook", "Mouse look"},
        {"centerview", "Center view"},
        {"cl_freelook", "Free look"},
        {"m_filter", "Smooth mouse"},
        {"sensitivity", "Mouse sensitivity"},
        {"+forward", "Move forward"},
        {"+back", "Move backward"},
        {"+moveleft", "Move left"},
        {"+moveright", "Move right"},
        {"+gostand", "Jump / stand"},
        {"gocrouch", "Crouch"},
        {"goprone", "Prone"},
        {"togglecrouch", "Toggle crouch"},
        {"toggleprone", "Toggle prone"},
        {"+movedown", "Crouch (hold)"},
        {"+prone", "Prone (hold)"},
        {"+stance", "Change stance"},
        {"+strafe", "Strafe"},
        {"+attack", "Fire weapon"},
        {"+toggleads_throw", "Aim down sights (toggle)"},
        {"+speed_throw", "Aim down sights (hold)"},
        {"+melee", "Melee"},
        {"weapnext", "Next weapon"},
        {"+reload", "Reload"},
        {"+sprint", "Sprint"},
        {"+breath_sprint", "Sprint / hold breath"},
        {"+holdbreath", "Hold breath"},
        {"+frag", "Frag grenade"},
        {"+smoke", "Special grenade"},
        {"+actionslot 3", "Weapon attachment"},
        {"+actionslot 4", "Equipment"},
        {"+actionslot 2", "Special equipment"},
        {"+activate", "Use / interact"},
        {"+actionslot 1", "Night vision"},
        {"screenshotjpeg", "Screenshot"},
        {"+scores", "Scoreboard"},
        {"vote yes", "Vote yes"},
        {"vote no", "Vote no"},
        {"+talk", "Voice chat (hold)"},
        {"chatmodepublic", "Global chat"},
        {"chatmodeteam", "Team chat"},
        {"mp_QuickMessage", "Quick messages"},
        {"ui_browserShowEmpty", "Empty servers"},
        {"ui_browserShowFull", "Full servers"},
        {"ui_browserShowPassword", "Password protected"},
        {"ui_browserShowDedicated", "Dedicated only"},
        {"ui_browserMod", "Modded servers"},
        {"ui_browserFriendlyfire", "Friendly fire"},
        {"ui_browserKillcam", "Killcam"},
        {"ui_browserShowPunkbuster", "PunkBuster"},
        {"ui_browserHardcore", "Hardcore"},
        {"ui_browserOldSchool", "Old school"},
    };
    if (dvar) for (const auto &label : labels) if (!I_stricmp(dvar, label.dvar)) return label.text;
    return dvar ? dvar : "Option";
}

static void CU_Conditions(itemDef_s *dst, const itemDef_s *src)
{
    dst->visibleExp = src->visibleExp;
    dst->enableDvar = src->enableDvar; dst->dvarTest = src->dvarTest; dst->dvarFlags = src->dvarFlags;
    dst->window.dynamicFlags[0] = src->window.dynamicFlags[0];
    const char *suffixes[]={"_edge","_icon"};
    for (const char *suffix : suffixes)
        if (auto *part=Menu_FindItemByName(dst->parent,va("%s%s",dst->window.name,suffix)))
        {
            part->visibleExp=src->visibleExp;
            part->window.dynamicFlags[0]=src->window.dynamicFlags[0];
            part->enableDvar=src->enableDvar; part->dvarTest=src->dvarTest; part->dvarFlags=src->dvarFlags;
        }
}

static void CU_Settings(UiContext *dc)
{
    const char *sourceNames[] = {"options_graphics", "options_graphics_texture", "options_sound", "options_voice",
        "options_game", "options_multi", "options_look", "options_move", "options_shoot", "options_misc", "controls_multi"};
    menuDef_t *sources[11];
    for (int i = 0; i < 11; ++i) sources[i] = Menus_FindByName(dc, sourceNames[i]);
    struct Page { const char *name; int source; int merge; int group; };
    const Page pages[] = {{"main_options",0,-1,0},{"options_graphics",0,-1,0},{"options_graphics_texture",1,-1,0},
        {"options_sound",2,3,1},{"options_voice",2,3,1},{"main_controls",6,-1,2},{"options_look",6,-1,2},
        {"options_move",7,-1,2},{"options_shoot",8,-1,2},{"options_misc",9,-1,2},{"controls_multi",10,-1,2},
        {"options_game",4,5,3},{"options_multi",4,5,3}};
    const char *tabNames[] = {"VIDEO", "AUDIO", "CONTROLS", "GAME & NETWORK"};
    const char *tabTargets[] = {"options_graphics", "options_sound", "options_look", "options_game"};
    for (const auto &page : pages)
    {
        Com_Printf(CON_CHANNEL_UI,"Theming settings page %s\n",page.name);
        menuDef_t *stock = sources[page.source];
        if (!stock) continue;
        auto *menu = CU_Menu(dc, stock, page.name, "SETTINGS", "Tune your experience. Related options live together.");
        if (page.merge >= 0 && sources[page.merge])
        {
            menu->onOpen = CU_Script(va("%s %s", stock->onOpen ? stock->onOpen : "", sources[page.merge]->onOpen ? sources[page.merge]->onOpen : ""));
            menu->onClose = CU_Script(va("%s %s", stock->onClose ? stock->onClose : "", sources[page.merge]->onClose ? sources[page.merge]->onClose : ""));
        }
        if (menu->onOpen)
        {
            std::string open=menu->onOpen;
            const std::string selfClose=std::string("\"close\" \"")+page.name+"\"";
            size_t found;
            while ((found=open.find(selfClose))!=std::string::npos)
            {
                size_t end=open.find(';',found);
                open.erase(found,end==std::string::npos?selfClose.size():end-found+1);
            }
            menu->onOpen=CU_Script(open.c_str());
        }
        menu->onESC = CU_Script("close self; close main_options; close main_controls; open main;");
        for (int tab = 0; tab < 4; ++tab)
            CU_Button(menu, va("cu_tab_%i",tab), tabNames[tab],
                va("close self; close main_options; close main_controls; open %s;", tabTargets[tab]), 26.0f + 149 * tab, 94, 143, page.group == tab);
        float contentY = 146;
        if (page.group == 0)
        {
            CU_Button(menu,"cu_display","DISPLAY & EFFECTS","close self; close main_options; open options_graphics;",26,129,180,page.source==0);
            CU_Button(menu,"cu_textures","TEXTURES","close self; close main_options; open options_graphics_texture;",212,129,135,page.source==1);
            contentY = 171;
        }
        if (page.group == 2)
        {
            const char *labels[] = {"MOUSE / LOOK","MOVEMENT","COMBAT","INTERACT","MULTIPLAYER"};
            const char *targets[] = {"options_look","options_move","options_shoot","options_misc","controls_multi"};
            for (int tab=0; tab<5; ++tab)
                CU_Button(menu,va("cu_bindtab_%i",tab),labels[tab],va("close self; close main_controls; open %s;",targets[tab]),26.0f+119*tab,129,113,page.source==tab+6);
            contentY = 174;
        }
        CU_Panel(menu,"cu_content",26,contentY-5,590,410-contentY);
        struct Field { menuDef_t *menu; int index; int row; };
        Field fields[64]; int fieldCount=0; int rowCount=0;
        for (int part=0; part<2; ++part)
        {
            int sourceIndex = part ? page.merge : page.source;
            if (sourceIndex < 0 || !sources[sourceIndex]) continue;
            auto *source = sources[sourceIndex];
            float lastY=-999;
            for (int i=0; i<source->itemCount && fieldCount<64; ++i)
            {
                auto *field=source->items[i];
                if (!field->dvar || !field->dvar[0] || field->window.rectClient.x < -100 || field->type==0) continue;
                // Stock variants at the same row retain their visibility conditions.
                float y=field->window.rectClient.y;
                if (y!=lastY) { ++rowCount; lastY=y; }
                fields[fieldCount++] = {source,i,rowCount-1};
            }
        }
        bool columns = rowCount > 8;
        int rows = columns ? (rowCount+1)/2 : rowCount;
        float spacing = rows > 8 ? 23.0f : 28.0f;
        for (int f=0; f<fieldCount; ++f)
        {
            const auto &field=fields[f];
            int col=columns && field.row>=rows ? 1:0;
            int row=col ? field.row-rows : field.row;
            float x=38.0f+col*295; float y=contentY+8+row*spacing;
            auto *label=CU_Label(menu,va("cu_setting_label_%i",f),CU_SourceLabel(field.menu,field.index),x,y,154,.205f);
            label->visibleExp=field.menu->items[field.index]->visibleExp;
            auto *control=CU_Control(menu,field.menu->items[field.index],va("cu_setting_%i",f),x+158,y,columns?111.0f:240.0f,22);
            control->textscale=.21f;
        }
        if (page.group==1 && sources[3])
        {
            CU_Label(menu,"cu_mic_label","MICROPHONE LEVEL",38,368,180,.19f);
            auto *meter=CU_Control(menu,sources[3]->items[82],"voicechat_level_indicator",210,372,240,9);
            meter->window.staticFlags |= 0x100000; meter->textaligny = 0;
        }
        for (int i=0; i<stock->itemCount; ++i)
        {
            auto *src=stock->items[i];
            if (src->window.name && (!I_stricmp(src->window.name,"apply") || !I_stricmp(src->window.name,"apply2")))
            {
                auto *apply=CU_Button(menu,src->window.name,"APPLY",src->action,506,420,110,true);
                CU_Conditions(apply,src);
            }
        }
        if (page.group==3 && sources[5])
        {
            for (int i=0;i<sources[5]->itemCount;++i)
                if (sources[5]->items[i]->action && strstr(sources[5]->items[i]->action,"cdkey_menu"))
                    CU_Button(menu,"cu_cdkey","PRODUCT KEY",sources[5]->items[i]->action,360,420,133);
            for(int i=0;i<stock->itemCount;++i)
                if (stock->items[i]->action && strstr(stock->items[i]->action,"language_restart_popmenu"))
                {
                    auto *apply=CU_Button(menu,"cu_language_apply","APPLY LANGUAGE",stock->items[i]->action,506,420,110,true);
                    CU_Conditions(apply,stock->items[i]);
                }
        }
        if (page.group==0)
            CU_Button(menu,"cu_autoconfig","AUTO CONFIG","open options_graphics_defaults;",130,420,135);
        if (page.group==2)
        {
            CU_Button(menu,"cu_reset","RESET CONTROLS","open options_control_defaults;",130,420,155);
            for(int i=0;i<stock->itemCount;++i)
                if (stock->items[i]->window.ownerDraw==250)
                    CU_Control(menu,stock->items[i],"keyBindStatus",295,418,310,24);
        }
        CU_Button(menu,"cu_back","BACK","close self; close main_options; close main_controls; open main;",26,420,90);
        CU_Replace(dc,menu);
    }
}


static void CU_ServerBrowser(UiContext *dc)
{
    auto *stock=Menus_FindByName(dc,"pc_join_unranked");
    if (!stock || stock->itemCount<84) return;
    auto *menu=CU_Menu(dc,stock,"pc_join_unranked","SERVER BROWSER","Find a match on the Internet, your local network or your favorites.");
    menu->onESC=CU_Script("close self; open main;");
    const char *labels[]={"INTERNET","LAN","FAVORITES"}; const int values[]={1,0,2};
    for(int i=0;i<3;++i)
    {
        auto *tab=CU_Button(menu,va("cu_source_%i",i),labels[i],
            va("setdvar ui_netSource %i; uiScript UpdateFilter; setitemcolor cu_sources backcolor .035 .052 .046 .93; setitemcolor cu_source_%i backcolor .12 .23 .1 .93;",values[i],i),26.0f+117*i,94,111, Dvar_GetInt("ui_netSource")==values[i]);
        tab->window.group="cu_sources";
    }
    CU_Label(menu,"cu_mode_label","MODE",389,99,65,.18f);
    CU_Control(menu,stock->items[39],"gametypefield",435,96,181,24);
    CU_Button(menu,"filterServers","FILTERS",stock->items[40]->action,26,130,100);
    CU_Button(menu,"createFavorite","NEW FAVORITE",stock->items[45]->action,133,130,125);
    CU_Button(menu,"findplayer","FIND PLAYER",stock->items[77]->action,265,130,112);
    CU_Button(menu,"refreshFilter","QUICK REFRESH",stock->items[43]->action,384,130,112);
    CU_Button(menu,"refreshSource","REFRESH LIST",stock->items[42]->action,503,130,113,true);
    CU_Panel(menu,"cu_server_panel",26,169,590,235);
    auto *list=CU_Control(menu,stock->items[58],"serverlist",26,191,590,211);
    list->textscale=.19f;
    for(int c=0;c<list->typeData.listBox->numColumns;++c)
    {
        auto &column=list->typeData.listBox->columnInfo[c];
        column.pos=(int)(column.pos*590.0f/626.0f);
        column.width=(int)(column.width*590.0f/626.0f);
    }
    const int headerIndices[]={59,61,63,64,65,66,67,69,71,73,75};
    const char *headerLabels[]={"PW","HW","SERVER","MAP","PLAYERS","MODE","VC","PURE","MOD","PB","PING"};
    for(int i=0;i<11;++i)
    {
        auto *original=stock->items[headerIndices[i]];
        float x=26+(original->window.rectClient.x-4)*590.0f/626.0f;
        auto *header=CU_Button(menu,original->window.name,headerLabels[i],original->action,x,169,original->window.rectClient.w*590.0f/626.0f,false,false);
        header->textscale=.15f; header->textalignx=3; header->textaligny=14;
        header->window.rectClient.h=20; header->window.group="grpTabs";
    }
    CU_Control(menu,stock->items[41],"refreshdate",290,405,325,14)->textscale=.16f;
    CU_Button(menu,"back","BACK","close self; open main;",26,426,65);
    auto *fav=CU_Button(menu,"addFavorite","FAVORITE",stock->items[78]->action,98,426,100); fav->visibleExp={}; fav->window.dynamicFlags[0]=4|0x10000; fav->dvarTest="ui_netSource"; fav->enableDvar="\"2\""; fav->dvarFlags=8; CU_Conditions(fav,fav);
    auto *del=CU_Button(menu,"delfavorite","REMOVE FAVORITE",stock->items[79]->action,98,426,100); del->visibleExp={}; del->window.dynamicFlags[0]=4|0x10000; del->dvarTest="ui_netSource"; del->enableDvar="\"2\""; del->dvarFlags=4; CU_Conditions(del,del);
    CU_Button(menu,"passwordenter","PASSWORD",stock->items[80]->action,205,426,94);
    CU_Button(menu,"serverinfo","SERVER INFO",stock->items[81]->action,306,426,110);
    CU_Button(menu,"accept","JOIN SERVER",stock->items[82]->action,497,426,119,true);
    CU_Replace(dc,menu);

    auto *filters=Menus_FindByName(dc,"filter_popmenu");
    if (filters)
    {
        auto *page=CU_Menu(dc,filters,"filter_popmenu","SERVER FILTERS","Refine the browser results. Changes are used by the existing server filters.");
        int count=0;
        for(int i=0;i<filters->itemCount;++i)
        {
            auto *src=filters->items[i];
            if (!src->dvar || !src->dvar[0]) continue;
            int col=count/5,row=count%5; float x=38.0f+col*295,y=133.0f+row*46;
            CU_Label(page,va("cu_filter_label_%i",count),CU_SourceLabel(filters,i),x,y,150,.21f);
            CU_Control(page,src,va("cu_filter_%i",count),x+154,y,115);
            ++count;
        }
        CU_Button(page,"cu_done","DONE",filters->items[69]->action,506,420,110,true);
        CU_Button(page,"cu_reset_filters","RESET FILTERS",filters->items[74]->action,26,420,140);
        CU_Replace(dc,page);
    }
}

static void CU_Mods(UiContext *dc)
{
    auto *stock=Menus_FindByName(dc,"mods_menu"); if (!stock || stock->itemCount<21) return;
    auto *menu=CU_Menu(dc,stock,"mods_menu","MOD LIBRARY","Browse installed mods and launch the selected experience.");
    menu->onESC=CU_Script("close self; open main;");
    CU_Panel(menu,"cu_library",26,100,359,302);
    CU_Label(menu,"cu_library_title","INSTALLED MODS",38,106,300,.22f);
    CU_Control(menu,stock->items[10],"modlist",34,137,343,257);
    CU_Panel(menu,"cu_mod_details",403,100,213,302);
    CU_Label(menu,"cu_mod_heading","SELECTED MOD",417,117,180,.26f);
    auto *selected=CU_Label(menu,"cu_selected_mod","",417,151,180,.22f); selected->text=nullptr; selected->dvar="ui_selectedMod";
    CU_Label(menu,"cu_mod_help1","Select a mod from your library.",417,197,185,.19f);
    CU_Label(menu,"cu_mod_help2","Launch reloads the game",417,231,185,.19f);
    CU_Label(menu,"cu_mod_help3","with that mod's content.",417,253,185,.19f);

    CU_Button(menu,"back","BACK","close self; open main;",26,420,90);
    CU_Button(menu,"accept","LAUNCH MOD",stock->items[15]->action,476,420,140,true);
    auto *clear=CU_Button(menu,"cu_unload_mod","UNLOAD MOD",stock->items[20]->action,319,420,140); CU_Conditions(clear,stock->items[20]);
    CU_Replace(dc,menu);
}


static statement_s CU_TransformExpression(const statement_s &source, float scale, float offset)
{
    if (!source.numEntries) return {};
    statement_s result={source.numEntries+6,(expressionEntry **)UI_Alloc((source.numEntries+6)*sizeof(expressionEntry *),4)};
    auto op=[](operationEnum value){auto *e=(expressionEntry *)UI_Alloc(sizeof(expressionEntry),4);memset(e,0,sizeof(*e));e->type=0;e->data.op=value;return e;};
    auto number=[](float value){auto *e=(expressionEntry *)UI_Alloc(sizeof(expressionEntry),4);memset(e,0,sizeof(*e));e->type=1;e->data.operand.dataType=VAL_FLOAT;e->data.operand.internals.floatVal=value;return e;};
    int n=0;result.entries[n++]=op(OP_LEFTPAREN);
    for(int i=0;i<source.numEntries;++i)result.entries[n++]=source.entries[i];
    result.entries[n++]=op(OP_RIGHTPAREN);result.entries[n++]=op(OP_MULTIPLY);result.entries[n++]=number(scale);
    result.entries[n++]=op(OP_ADD);result.entries[n++]=number(offset);
    return result;
}

static void CU_Barracks(UiContext *dc)
{
    auto *stock=Menus_FindByName(dc,"menu_challenges"); if (!stock || stock->itemCount<184) return;
    for(int page=0;page<2;++page)
    {
        auto *menu=CU_Menu(dc,stock,page?"cu_challenges":"menu_challenges","BARRACKS","Your rank, progression, combat record and challenge unlocks.");
        menu->onESC=CU_Script("close self; open main;");
        CU_Button(menu,"cu_progress","PROGRESS","close self; open menu_challenges;",26,94,135,!page);
        CU_Button(menu,"cu_challenges_tab","CHALLENGES","close self; open cu_challenges;",168,94,135,page);
        CU_Panel(menu,"cu_rank_panel",306,132,310,271);
        for(int i=147;i<184;++i)
        {
            auto *src=stock->items[i]; const auto &r=src->window.rectClient;
            if ((r.w<=0 || r.h<=0) && !src->rectWExp.numEntries && !src->rectHExp.numEntries) continue;
            auto *item=CU_Item(menu,va("cu_rank_%i",i),0,0,1,1); *item=*src;
            item->parent=menu; item->window.name=String_Alloc(va("cu_rank_%i",i));
            CU_Rect(item,312+(r.x+354)*1.02f,136+(r.y-34)*.77f,r.w*1.02f,r.h*.77f);
            item->textalignx*=1.02f; item->textaligny*=.77f; item->textscale=.21f;
            item->rectXExp=CU_TransformExpression(src->rectXExp,1.02f,312+354*1.02f);
            item->rectYExp=CU_TransformExpression(src->rectYExp,.77f,136-34*.77f);
            item->rectWExp=CU_TransformExpression(src->rectWExp,1.02f,0);
            item->rectHExp=CU_TransformExpression(src->rectHExp,.77f,0);
            if (src->text || src->textExp.numEntries) CU_Color(item->window.foreColor,.78f,.88f,.72f);
            if (r.h<=1) CU_Color(item->window.foreColor,.22f,.31f,.25f,.6f);
        }
        CU_Panel(menu,"cu_progress_panel",26,132,262,271);
        if(page)
        {
            const int unlocked[]={40,48,56,64,72,81,89,97,105,113};
            const int locked[]={43,51,59,67,75,84,92,100,108,116};
            const char *labels[]={"ASSAULT","SMG","LMG","SHOTGUNS","SNIPERS","BOOT CAMP","OPERATIONS","KILLER","HUMILIATION","ELITE"};
            for(int row=0;row<10;++row)
                for(int variant=0;variant<2;++variant)
                {
                    auto *src=stock->items[variant?locked[row]:unlocked[row]];
                    auto *button=CU_Button(menu,va("cu_tier_%i_%i",row,variant),variant?va("%s / LOCKED",labels[row]):labels[row],src->action,36,141+row*25.0f,242);
                    button->window.rectClient.h=23; button->textaligny=15; button->textscale=.20f;
                    CU_Conditions(button,src);
                }
        }
        else
        {
            CU_Label(menu,"cu_rank_intro","YOUR COMBAT RECORD",42,151,230,.24f);
            CU_Label(menu,"cu_rank_desc1","Earn XP through multiplayer",42,198,230,.20f);
            CU_Label(menu,"cu_rank_desc2","matches and challenges.",42,220,230,.20f);
            CU_Label(menu,"cu_rank_desc3","Track your next rank and",42,265,230,.20f);
            CU_Label(menu,"cu_rank_desc4","browse challenge categories",42,287,230,.20f);
            CU_Label(menu,"cu_rank_desc5","as you unlock them.",42,309,230,.20f);
            CU_Button(menu,"cu_browse_challenges","VIEW CHALLENGES","close self; open cu_challenges;",42,354,228,true);
        }
        CU_Button(menu,"back","BACK","close self; open main;",26,420,90);
        CU_Replace(dc,menu);
    }
}



static const char *CU_RuleLabel(const char *dvar)
{
    struct Rule { const char *key; const char *label; };
    const Rule labels[]={{"timelimit","Time limit"},{"scorelimit","Score limit"},{"roundlength","Round length"},
        {"bombtimer","Bomb timer"},{"planttime","Plant time"},{"defusetime","Defuse time"},{"roundswitch","Round switch"},
        {"oldschool","Old school"},{"hardcore","Hardcore"},{"spectatetype","Spectating"},{"allowkillcam","Killcam"},
        {"fftype","Friendly fire"},{"destroytime","HQ hold time"},{"spawntime","HQ respawn delay"},{"numlives","Lives"}};
    for(const auto &label:labels) if(strstr(dvar,label.key))return label.label;
    return dvar;
}

static void CU_GameRules(UiContext *dc)
{
    const char *names[]={"settings_quick_war","settings_quick_sd","settings_quick_sab","settings_quick_dom","settings_quick_dm","settings_quick_koth","settings_quick_ctf"};
    for(const char *name:names)
    {
        auto *stock=Menus_FindByName(dc,name); if(!stock || stock->itemCount<16)continue;
        auto *menu=CU_Menu(dc,stock,name,"GAME MODE RULES","Adjust the selected mode. Changes are used when your session starts.");
        int fields[32],count=0;
        for(int i=0;i<stock->itemCount && count<32;++i)
            if(stock->items[i]->dvar && stock->items[i]->dvar[0] && stock->items[i]->type)fields[count++]=i;
        int rows=count>8?(count+1)/2:count;
        CU_Panel(menu,"cu_rules_panel",26,111,590,291);
        for(int i=0;i<count;++i)
        {
            int col=i>=rows?1:0,row=i%rows;float x=38.0f+col*295,y=128.0f+row*32;
            auto *src=stock->items[fields[i]];
            CU_Label(menu,va("cu_rule_label_%i",i),CU_RuleLabel(src->dvar),x,y,150,.22f);
            CU_Control(menu,src,src->window.name&&src->window.name[0]?src->window.name:va("cu_rule_%i",i),x+153,y,count>8?117.0f:220.0f);
        }
        CU_Button(menu,"back","BACK","close self;",26,420,90);
        CU_Button(menu,"reset_to_default","RESET RULES",stock->items[15]->action,460,420,156);
        CU_Replace(dc,menu);
    }
}

// Small dialogs keep their native geometry, expressions, validation and scripts.
// Only the presentation is copied into UI-owned items and recolored.
static void CU_Dialogs(UiContext *dc)
{
    const int count=dc->menuCount;
    for(int m=0;m<count;++m)
    {
        auto *stock=dc->Menus[m]; const char *name=stock->window.name;
        if (!name || stock->itemCount>=250) continue;
        bool dialog=strstr(name,"popmenu") || strstr(name,"popup") || (!strncmp(name,"settings_quick_",15) && strchr(name+15,'_'))
            || !strcmp(name,"cdkey_menu") || !strcmp(name,"options_control_defaults") || !strcmp(name,"options_graphics_defaults");
        if (!dialog || !strcmp(name,"filter_popmenu")) continue;
        auto *menu=(menuDef_t *)UI_Alloc(sizeof(menuDef_t),4); *menu=*stock;
        menu->items=(itemDef_s **)UI_Alloc(256*sizeof(itemDef_s *),4); menu->itemCount=0;
        CU_Color(menu->focusColor,.72f,.94f,.58f); CU_Color(menu->disableColor,.38f,.43f,.39f);
        for(int i=0;i<stock->itemCount;++i)
        {
            auto *src=stock->items[i];
            auto *item=(itemDef_s *)UI_Alloc(sizeof(itemDef_s),4); *item=*src; item->parent=menu;
            menu->items[menu->itemCount++]=item;
            const char *material=src->window.background?src->window.background->info.name:"";
            if (strstr(material,"button_highlight_end"))
            {
                item->window.background=nullptr; item->window.style=0;
                item->window.dynamicFlags[0]&=~4; item->visibleExp={};
            }
            else if (strstr(material,"gradient_fadein") || !strcmp(material,"white"))
            {
                item->window.background=nullptr; item->window.style=1;
                CU_Color(item->window.backColor,.025f,.043f,.034f,src->window.rectClient.h>40?.96f:.74f);
                CU_Color(item->window.borderColor,.24f,.34f,.25f,.8f);
            }
            CU_Color(item->window.foreColor,.84f,.89f,.83f);
            if (!item->text && !item->textExp.numEntries && item->type==0)
                CU_Color(item->window.foreColor,.025f,.043f,.034f,.88f);
            item->textStyle=0;
            if (item->type==6 && src->typeData.listBox)
            {
                auto *list=(listBoxDef_s *)UI_Alloc(sizeof(listBoxDef_s),4); *list=*src->typeData.listBox;
                CU_Color(list->selectBorder,.42f,.65f,.34f,.85f); item->typeData.listBox=list;
            }
            if (item->type==1 && !(item->window.staticFlags&0x100000) && item->action && item->window.rectClient.w>=40 && item->window.rectClient.h<=30)
            {
                item->window.name=String_Alloc(src->window.name&&src->window.name[0]?src->window.name:va("cu_dialog_button_%i",i));
                item->window.background=nullptr; item->window.style=1; item->window.border=1; item->window.borderSize=.5f;
                CU_Color(item->window.backColor,.035f,.052f,.047f,.72f); CU_Color(item->window.borderColor,.20f,.25f,.23f,.7f);
                item->onFocus=CU_Script(va("%s setitemcolor %s backcolor .13 .22 .115 .83; setitemcolor %s bordercolor .62 .87 .48 1;",src->onFocus?src->onFocus:"",item->window.name,item->window.name));
                item->leaveFocus=CU_Script(va("%s setitemcolor %s backcolor .035 .052 .047 .72; setitemcolor %s bordercolor .20 .25 .23 .7;",src->leaveFocus?src->leaveFocus:"",item->window.name,item->window.name));
            }
        }
        CU_Replace(dc,menu);
    }
}


static void CU_MainAlias(UiContext *dc)
{
    // Legacy dialogs return to main_text, which originally contained all stock buttons.
    auto *menu=(menuDef_t *)UI_Alloc(sizeof(menuDef_t),4); *menu=*s_customMainMenu;
    menu->window.name="main_text";
    menu->onOpen=CU_Script("focusfirst;");
    menu->items=(itemDef_s **)UI_Alloc(menu->itemCount*sizeof(itemDef_s *),4);
    for(int i=0;i<menu->itemCount;++i)
    {
        auto *item=(itemDef_s *)UI_Alloc(sizeof(itemDef_s),4); *item=*s_customMainMenu->items[i]; item->parent=menu;
        menu->items[i]=item;
    }
    CU_Replace(dc,menu);
}


static void CU_FrontendSmoke(UiContext *dc)
{
    if (!s_cuFrontendThemed || !Dvar_GetBool("ui_frontendSmoke")) return;
    Dvar_SetBoolByName("ui_frontendSmoke",false);
    int checks=0,failed=0;
    auto check=[&](bool pass,const char *label){++checks;if(!pass)++failed;Com_Printf(CON_CHANNEL_UI,"Frontend smoke %s: %s\n",pass?"PASS":"FAIL",label);};
    auto *local=Menus_FindByName(dc,"createserver");
    Menus_OpenByName(dc,"createserver");
    Item_RunScript(dc,Menu_FindItemByName(local,"cu_advanced"),(char *)Menu_FindItemByName(local,"cu_advanced")->action);
    auto *advanced=Menus_FindByName(dc,"cu_local_advanced");
    check(Menu_IsVisible(dc,advanced)&&!Menu_IsVisible(dc,local),"Session to Advanced navigation");
    Item_RunScript(dc,Menu_FindItemByName(advanced,"cu_basic"),(char *)Menu_FindItemByName(advanced,"cu_basic")->action);
    check(Menu_IsVisible(dc,local)&&!Menu_IsVisible(dc,advanced),"Advanced to Session navigation");
    auto toggle=[&](menuDef_t *menu,const char *dvar,const char *label)
    {
        for(int i=0;i<menu->itemCount;++i)
        {
            auto *item=menu->items[i];
            if(item->dvar && !strcmp(item->dvar,dvar))
            {
                std::string before=Dvar_GetVariantString(dvar);
                item->window.dynamicFlags[0]|=2;
                Item_YesNo_HandleKey(dc,item,K_ENTER);
                check(before!=Dvar_GetVariantString(dvar),label);
                Dvar_SetFromStringByName(dvar,before.c_str());
                item->window.dynamicFlags[0]&=~2;
                return;
            }
        }
        check(false,label);
    };
    toggle(local,"sv_voice","Local voice toggle updates its dvar (restored)");
    int oldMap=Dvar_GetInt("ui_currentMap"),oldNetMap=Dvar_GetInt("ui_currentNetMap");
    if(UI_FeederCount(0,4.0f)>1)
    {
        UI_FeederSelection(0,4.0f,1);
        check(Dvar_GetInt("ui_currentNetMap")!=oldNetMap,"Map selection callback updates selected map (restored)");
        Dvar_SetIntByName("ui_currentMap",oldMap);Dvar_SetIntByName("ui_currentNetMap",oldNetMap);
    }
    int customMaps = 0;
    char userMapPath[512];
    for (int i = 0; i < sharedUiInfo.mapCount; ++i)
        if (FS_UserMapZonePath(sharedUiInfo.mapList[i].mapLoadName, userMapPath, sizeof(userMapPath))) ++customMaps;
    check(customMaps > 0 && sharedUiInfo.mapCount <= 128, "Installed custom maps are in the bounded Local Play catalog");
    int customRow = 0;
    for (int i = 0; i < sharedUiInfo.mapCount; ++i)
    {
        if (!sharedUiInfo.mapList[i].active) continue;
        if (FS_UserMapZonePath(sharedUiInfo.mapList[i].mapLoadName, userMapPath, sizeof(userMapPath)))
        {
            UI_FeederSelection(0, 4.0f, customRow);
            check(Dvar_GetInt("ui_currentNetMap") == i, "Local Play selects an installed custom map through its native feeder");
            check(UI_GetLevelShot(i) != nullptr, "Custom-map preview resolves without a missing material");
            break;
        }
        ++customRow;
    }
    Dvar_SetIntByName("ui_currentMap",oldMap);Dvar_SetIntByName("ui_currentNetMap",oldNetMap);
    FS_SetUserMap("");
    Menus_CloseByName(dc,"createserver");
    auto *audio=Menus_FindByName(dc,"options_sound");Menus_OpenByName(dc,"options_sound");
    toggle(audio,"cl_voice","Merged audio voice toggle updates its dvar (restored)");
    auto *gameTab=Menu_FindItemByName(audio,"cu_tab_3");Item_RunScript(dc,gameTab,(char *)gameTab->action);
    auto *game=Menus_FindByName(dc,"options_game");
    check(Menu_IsVisible(dc,game)&&!Menu_IsVisible(dc,audio),"Audio to merged Game & Network navigation");
    auto *controlTab=Menu_FindItemByName(game,"cu_tab_2");Item_RunScript(dc,controlTab,(char *)controlTab->action);
    check(Menu_IsVisible(dc,Menus_FindByName(dc,"options_look")),"Settings Controls tab navigation");
    Menus_CloseByName(dc,"options_look");
    Menus_OpenByName(dc,"mods_menu");
    int oldMod=sharedUiInfo.modIndex;
    if(sharedUiInfo.modCount>1)
    {
        UI_FeederSelection(0,9.0f,1);
        check(sharedUiInfo.modIndex==1 && !strcmp(Dvar_GetString("ui_selectedMod"),sharedUiInfo.modList[1].modName),"Mod selection and displayed name agree (restored)");
        UI_FeederSelection(0,9.0f,oldMod);
    }
    Menus_CloseByName(dc,"mods_menu");
    auto *browser=Menus_FindByName(dc,"pc_join_unranked");int source=Dvar_GetInt("ui_netSource");
    Menus_OpenByName(dc,"pc_join_unranked");
    auto *favorites=Menu_FindItemByName(browser,"cu_source_2");Item_RunScript(dc,favorites,(char *)favorites->action);
    check(Dvar_GetInt("ui_netSource")==2,"Favorites tab switches browser source (restored)");
    Dvar_SetIntByName("ui_netSource",source);Menus_CloseByName(dc,"pc_join_unranked");
    int bindings=0;
    const char *names[]={"options_look","options_move","options_shoot","options_misc","controls_multi"};
    for(const char *name:names)
    {
        auto *menu=Menus_FindByName(dc,name);
        for(int i=0;i<menu->itemCount;++i)if(menu->items[i]->type==14 && menu->items[i]->dvar)++bindings;
    }
    check(bindings>=30,"Binding widgets retained across all five control categories");
    Com_Printf(CON_CHANNEL_UI,"Frontend smoke result: %i checks, %i failures\n",checks,failed);
}

static void CU_HostMapSmoke(UiContext *dc)
{
    if (!Dvar_GetBool("ui_hostMapSmoke")) return;
    Dvar_SetBoolByName("ui_hostMapSmoke", false);
    const std::string requested = Dvar_GetString("ui_hostMapName");
    Menus_OpenByName(dc, "createserver");
    Dvar_SetIntByName("ui_dedicated", 0);
    Dvar_SetBoolByName("sv_punkbuster", false);
    Dvar_SetBoolByName("cl_punkbuster", false);
    int row = 0;
    for (int i = 0; i < sharedUiInfo.mapCount; ++i)
    {
        if (!sharedUiInfo.mapList[i].active) continue;
        if (requested == sharedUiInfo.mapList[i].mapLoadName)
        {
            UI_FeederSelection(0, 4.0f, row);
            auto *menu = Menus_FindByName(dc, "createserver");
            auto *start = Menu_FindItemByName(menu, "accept");
            Com_Printf(CON_CHANNEL_UI, "Local Play host smoke: starting %s through Start Session\n", requested.c_str());
            Item_RunScript(dc, start, (char *)start->action);
            return;
        }
        ++row;
    }
    Com_PrintError(CON_CHANNEL_UI, "Local Play host smoke: requested map is not selectable\n");
}

static void UI_ThemeFrontend(UiContext *dc)
{
    s_cuFrontendThemed = false;
    if (!s_customMainMenu || g_mapname[0] || Dvar_GetString("fs_game")[0]) return;
    Dvar_RegisterString("ui_selectedMod","",DVAR_NOFLAG,"Selected mod in the front end");
    CU_MainAlias(dc);
    Com_Printf(CON_CHANNEL_UI,"Theming Local Play\n"); CU_LocalPlay(dc);
    Com_Printf(CON_CHANNEL_UI,"Theming Settings\n"); CU_Settings(dc);
    Com_Printf(CON_CHANNEL_UI,"Theming Server Browser\n"); CU_ServerBrowser(dc);
    Com_Printf(CON_CHANNEL_UI,"Theming Mods\n"); CU_Mods(dc);
    Com_Printf(CON_CHANNEL_UI,"Theming Barracks\n"); CU_Barracks(dc);
    CU_GameRules(dc);
    CU_Dialogs(dc);
    s_cuFrontendThemed = true;
}
