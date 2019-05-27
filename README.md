# csgo-cvar-unhide

This Counter-Strike: Global Offensive plugin reveals all console variables (convars/cvars) that are marked as hidden or development-only. The plugin also has functionality to set console variables to arbitrary values despite any hard-coded minimum/maximum. Furthermore, console commands can be 'force dispatched' regardless of whether they are marked as hidden, cheat or development-only.

A list of all console variables/commands that are made available by this plugin can be found in [cvarlist.md](./cvarlist.md).

✔️ Note: despite being called a server plugin, it can be loaded client-side. However, you must add `-insecure` to the launch options for the plugin to load.

## Installation

1. Download the latest release of the plugin from GitHub: https://github.com/saul/csgo-cvar-unhide/releases/latest
2. Extract the contents of the ZIP to `$STEAM\steamapps\common\Counter-Strike Global Offensive\csgo`.
3. Start Counter-Strike: Global Offensive with `-insecure` in the launch options. If you do not know how to do this, take a look this [Steam Community guide](https://steamcommunity.com/sharedfiles/filedetails/?id=379782151).

## Available commands

If you installed the plugin correctly, you should now be able to use the following commands in the console:

- **cvar_unhide_all**: Unhide all FCVAR_HIDDEN and FCVAR_DEVELOPMENTONLY convars
- **find_all**: Replica of "find". Ignores FCVAR_HIDDEN or FCVAR_DEVELOPMENTONLY flags
- **cvarlist_all**: List all ConVars. Syntax: [hidden]
- **force_dispatch**: Dispatch a command regardless of any hidden, cheat or developmentonly flags
- **cvar_set**: Set the value of a ConVar regardless of its maximum/minimum values
- **dump_netprops**: Dump all network props. Syntax: [table depth = 1]. A table depth of -1 indicates infinite depth.