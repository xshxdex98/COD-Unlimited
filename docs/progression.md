# Progression

How players level up in COD Unlimited, what they unlock, and where their progress lives.
This is the plan the code is built against; change it here first when the design changes.

## Player level and prestige

- **Levels 1-99.** Levels 1-55 keep CoD4's XP curve (120,280 XP to reach 55), so the early game
  feels familiar. From 56 to 99 each level costs 120 XP more than the one before it, continuing
  CoD4's last step (473,530 XP to max). `tools/ranks.py` builds the table, so tuning from real
  match data is a one-line change there.
- **Prestige 1-10.** At level 99 with the bar full, the player can prestige: back to level 1,
  with a new prestige icon, a prestige camo and an unlock token. Weapon levels, camos, gloves
  and challenges are kept. Only the level resets, as in modern CoD.
- **Unlimited (after prestige 10).** No more resets. Players earn Unlimited levels 1-1000 at a flat
  XP cost each. Every 100 levels gives a new icon tier, and milestones give special camos and
  gloves. It's the long tail for people who live in the game, and it gives the project its name.
- **Unlock tokens.** Each prestige gives one. A token unlocks any gun or perk early, so a fresh
  prestige can still use a favourite gun from level 1.

## What unlocks what

| Unlock | Earned by |
| --- | --- |
| Guns, perks, equipment, killstreaks | Player level, or an unlock token |
| Attachments | That gun's weapon level |
| Standard camos | That gun's camo challenges |
| Mastery camos | Finishing every standard camo on a gun, then on a whole class, then on everything |
| Prestige camos | Prestiging (one per prestige) and Unlimited milestones |
| Hidden camos | Secrets: odd challenges, map easter eggs, zombies |
| Gloves and arms | Prestige, Unlimited milestones, challenges and secrets |
| Calling cards and emblems | Rewards along all of the above |

## Weapon levels

- Every gun has its own level, 1 to its max (around 30 for stock guns; set per gun in its row).
- Weapon XP comes from kills with that gun, plus bonuses for headshots, longshots and
  multikills, and a share of the match bonus for the gun used most.
- Attachments unlock at set weapon levels, listed per gun. New guns and attachments are just new
  rows, so adding weapons never needs code changes.
- A level-up shows in the match ("M4 level 12: ACOG unlocked"), not only after it.

## Camos (40+)

Camos are applied by the renderer to any gun (step 2), so one camo works across every weapon,
including ones added later.

- **Standard: about 20 per gun**, in groups of four. Each camo is one challenge on that gun:
  kills, headshots, melee kills, longshots, hipfire kills, crouched/prone kills, kills while
  aiming, multikills, point-blank kills, kills with no attachments, and so on. A group opens
  once the one before it is done.
- **Mastery: 4 tiers.** Gold for one gun, a class-wide tier when every gun in a class is gold,
  then two more over all guns. Each has its own name and look; we don't reuse Activision's
  names.
- **Prestige: 10, plus Unlimited milestone camos.**
- **Hidden: 6 or more**, kept out of the menus until earned.

Challenge targets and rewards are rows in a table, like everything else here.

## Gloves and arms

Viewmodel arms are a cosmetic the player picks in the loadout. The server applies the choice
when it spawns the player. Unlocks come from the table above. Faction arms stay the default.

## The modern flow

- **XP from everything:** kills, assists, objectives, medals, streaks and challenges, each with
  a popup and a short sound, as in modern CoD.
- **After-action report** after each match: XP earned and its breakdown, the level bar filling,
  weapon XP per gun used, and the unlocks earned.
- **Loadout:** 10 classes, with attachments, camo, reticle and gloves picked per gun. Locked
  items say how to unlock them.
- **Barracks:** prestige, Unlimited level, the camo grid per gun with progress bars, gloves,
  calling cards and stats.
- **Daily and weekly challenges and double-XP weekends** come from the Hub, so they change
  without a patch.

## Shared progress: the Hub

Progress follows the player to every COD Unlimited server, whoever hosts it. That means one
central service, the Hub, holds everyone's progress. Servers and clients talk to it over HTTPS.

- **Players sign in once with Steam** (Steam OpenID in the browser, which needs no publisher
  key). The Hub gives the game a token, and every request after that uses it.
- **Servers report what happened, not results.** A server sends events such as "player X killed
  Y with the M4, headshot, 41 m". The Hub turns events into XP, weapon XP and challenge progress
  with the same tables the game ships. A server can't send "give player X level 99".
- **Anyone can host, and every server is checked.** A server gets a key the first time it starts.
  The Hub caps how much any player can earn per minute and per match, and flags servers whose
  numbers don't look human. A flagged or abusive server loses its key and goes unranked: its
  matches still play, they just don't count. The server browser shows which servers are ranked.
- **Clients read their progress from the Hub,** so the menus show it at the main menu, before
  joining anything. In a match, the server sends unlock changes to the client as they happen.
- **Offline and LAN** play uses a local store with the same format. That progress stays local
  and is never sent to the Hub.
- **Modes adapted from community mods** opt in to progression through a small script API:
  award XP, add to a challenge, check whether something is unlocked. A mode that doesn't
  call it simply doesn't rank.

The engine already sends rank and prestige to clients as 8 bits each (0-255), so 99 levels,
10 prestiges and an Unlimited tier fit without a protocol change.

## Build order

1. Tables: player levels, prestige, guns and attachments, camos and challenges, gloves.
2. The local progression store and the script API, so modes and the rank code run against it.
3. The rank system rewrite on top of them: XP events, popups, level-ups, prestige.
4. The Hub service, then switching servers and clients from the local store to the Hub.
5. Menus: after-action report, loadout, barracks, camo grid.
6. Camo rendering (step 2) turns the earned camos into visible ones.
