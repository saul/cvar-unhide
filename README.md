# cvar-unhide [![CI](https://github.com/saul/cvar-unhide/actions/workflows/ci.yml/badge.svg)](https://github.com/saul/cvar-unhide/actions/workflows/ci.yml)

This Source Engine plugin reveals all console variables (convars/cvars) that are marked as hidden or development-only. The plugin can also set console variables to arbitrary values, bypassing any hard-coded minimum/maximum. Furthermore, console commands can be 'force dispatched' regardless of whether they are marked as hidden, cheat or development-only.

A list of all console variables/commands that are made available by this plugin can be found in [cvarlist-csgo.md](./cvarlist-csgo.md) and [cvarlist-portal2.md](./cvarlist-portal2.md).

> ## 💡
>
> You must add `-insecure` to CSGO's launch options for the game client to load plugins.
> The plugin will not load without this command-line argument set.

## Supported games

- Counter-Strike: Global Offensive
- Portal 2

## Installation

1. **Download the latest release of the plugin**. Choose the correct release for your game and OS: \
   https://github.com/saul/cvar-unhide/releases/latest
1. **Extract the contents of the ZIP to the game's mod folder.**

   - 📂 `$STEAM\steamapps\common\Counter-Strike Global Offensive\csgo` for CSGO
   - 📂 `$STEAM\steamapps\common\Portal 2\portal2` for Portal 2

   After extraction there should be an `addons` folder in the game folder, e.g. `Portal 2\portal2\addons\...`

1. **Start the game from Steam.** \
   ⚠ CSGO must be launched with `-insecure` in the launch options. If you don't know how to do this, take a look this [Steam Community guide](https://steamcommunity.com/sharedfiles/filedetails/?id=379782151).

## Available commands

If you installed the plugin correctly, you should now be able to use the following commands in the console:

- **cvar_set**: Set the value of a ConVar regardless of its maximum/minimum values
- **cvar_unhide_all**: Unhide all FCVAR_HIDDEN and FCVAR_DEVELOPMENTONLY convars
- **cvarlist_all**: List all ConVars. Syntax: [hidden]
- **dump_netprops**: Dump all network props. Syntax: [table depth = 1]. A table depth of -1 indicates infinite depth.
- **find_all**: Replica of "find". Ignores FCVAR_HIDDEN or FCVAR_DEVELOPMENTONLY flags
- **force_dispatch**: Dispatch a command regardless of any hidden, cheat or developmentonly flags
- **hltv_modevents**: List all HLTV mod events (game events that are recorded in demos).
