//========= Copyright (C) 2016, Saul Rennison, All rights reserved. ===========
//
// Written: October 2016
// Author: Saul Rennison
//
//=============================================================================

#include <stdio.h>
#include <map>
#include <vector>
#include <string>

#include "interface.h"
#include "dbg.h"
#include "tier1/tier1.h"
#include "engine/iserverplugin.h"
#include "ihltvdirector.h"
#include "eiface.h"
#include "convar.h"
#include "Color.h"
#include "fmtstr.h"
#include "server_class.h"
#include "version.h"

//-----------------------------------------------------------------------------
// Colours
//-----------------------------------------------------------------------------
#define COLOUR_YELLOW	Color(255,255,0,255)
#define COLOUR_CYAN		Color(0,255,255,255)
#define COLOUR_GREEN	Color(0,255,0,255)

//-----------------------------------------------------------------------------
// Interface globals
//-----------------------------------------------------------------------------
CreateInterfaceFn g_gameServerFactory = NULL;
IServerGameDLL *serverGameDll = NULL;
IHLTVDirector *hltvdirector = NULL;

//-----------------------------------------------------------------------------
// Purpose: interface load helper
//-----------------------------------------------------------------------------
template<typename T>
bool LoadInterface(T*& pResult, CreateInterfaceFn factory, const char *pszInterfaceName, int nVersionStart)
{
	Msg("cvar-unhide: Loading %s%.3d ", pszInterfaceName, nVersionStart);

	for (int i = nVersionStart; i < nVersionStart + 5; ++i)
	{
		CFmtStr strInterfaceString("%s%.3d", pszInterfaceName, i);
		
		if (i > nVersionStart)
			Msg("  - trying %s... ", strInterfaceString.Access());

		int nResult = IFACE_FAILED;
		T* pInterface = (T*)factory(strInterfaceString, &nResult);

		if(nResult == IFACE_OK)
		{
			ConColorMsg(COLOUR_GREEN, "OK\n");
			pResult = pInterface;
			return true;
		}

		Warning("FAILED\n");
	}

	Warning("  - giving up.\n");
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CServerPlugin : public IServerPluginCallbacks
{
public:
	CServerPlugin();
	~CServerPlugin();

	// IServerPluginCallbacks methods
public:
	virtual bool			Load( CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory );
	virtual void			Unload();
	virtual void			Pause() {};
	virtual void			UnPause() {};
	virtual const char		*GetPluginDescription( void );
	virtual void			LevelInit( const char *pMapName );
	virtual void			ServerActivate( edict_t *pEdictList, int edictCount, int clientMax ) {};
	virtual void			GameFrame( bool simulating ) {};
	virtual void			LevelShutdown();
	virtual void			ClientActive( edict_t *pEntity ) {};
	virtual void			ClientFullyConnect( edict_t *pEntity ) {};
	virtual void			ClientDisconnect( edict_t *pEntity ) {};
	virtual void			ClientPutInServer( edict_t *pEntity, const char *playername ) {};
	virtual void			SetCommandClient( int index );
	virtual void			ClientSettingsChanged( edict_t *pEdict ) {};
	virtual PLUGIN_RESULT	ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen ) { return PLUGIN_CONTINUE; }
	virtual PLUGIN_RESULT	ClientCommand( edict_t *pEntity, const CCommand &args ) { return PLUGIN_CONTINUE; }
	virtual PLUGIN_RESULT	NetworkIDValidated( const char *pszUserName, const char *pszNetworkID ) { return PLUGIN_CONTINUE; }
	virtual void			OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue ) {};
	virtual void			OnEdictAllocated( edict_t *edict ) {};
	virtual void			OnEdictFreed( const edict_t *edict ) {};
	virtual bool			BNetworkCryptKeyCheckRequired(uint32 unFromIP, uint16 usFromPort, uint32 unAccountIdProvidedByClient,
		bool bClientWantsToUseCryptKey) { return false; };
	virtual bool			BNetworkCryptKeyValidate(uint32 unFromIP, uint16 usFromPort, uint32 unAccountIdProvidedByClient,
		int nEncryptionKeyIndexFromClient, int numEncryptedBytesFromClient, byte* pbEncryptedBufferFromClient,
		byte* pbPlainTextKeyForNetchan) { return false; }

	// Internal interface
public:
	int						GetCommandIndex() { return m_iClientCommandIndex; }

private:
	int m_iClientCommandIndex;
};

//-----------------------------------------------------------------------------
// Singleton accessor
//-----------------------------------------------------------------------------
CServerPlugin g_Plugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CServerPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_Plugin);

//-----------------------------------------------------------------------------
// Purpose: constructor/destructor
//-----------------------------------------------------------------------------
CServerPlugin::CServerPlugin()
{
	m_iClientCommandIndex = 0;
}

CServerPlugin::~CServerPlugin()
{
}

//-----------------------------------------------------------------------------
// Purpose: called when the plugin is loaded, load the interface we need from the engine
//-----------------------------------------------------------------------------
bool CServerPlugin::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
	ConnectTier1Libraries(&interfaceFactory, 1);

	g_gameServerFactory = gameServerFactory;

	MathLib_Init();
	ConVar_Register();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: called when the plugin is unloaded (turned off)
//-----------------------------------------------------------------------------
void CServerPlugin::Unload()
{
	ConVar_Unregister();
	DisconnectTier1Libraries();
}

//-----------------------------------------------------------------------------
// Purpose: the name of this plugin, returned in "plugin_print" command
//-----------------------------------------------------------------------------
const char *CServerPlugin::GetPluginDescription()
{
	return "cvar-unhide, " CVAR_UNHIDE_VERSION ", Saul Rennison";
}

//-----------------------------------------------------------------------------
// Purpose: called on level start
//-----------------------------------------------------------------------------
void CServerPlugin::LevelInit(const char *pMapName)
{
}

//-----------------------------------------------------------------------------
// Purpose: called on level end (as the server is shutting down or going to a new map)
//-----------------------------------------------------------------------------
void CServerPlugin::LevelShutdown()
{
}

//-----------------------------------------------------------------------------
// Purpose: called on level start
//-----------------------------------------------------------------------------
void CServerPlugin::SetCommandClient( int index )
{
	m_iClientCommandIndex = index;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CON_COMMAND(cvar_unhide_all, "Unhide all FCVAR_HIDDEN and FCVAR_DEVELOPMENTONLY convars")
{
	ICvar::Iterator iter(g_pCVar);

	int nUnhidden = 0;

	for (iter.SetFirst(); iter.IsValid(); iter.Next())
	{
		ConCommandBase *cmd = iter.Get();

		if (cmd->IsFlagSet(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN))
			nUnhidden++;

		cmd->RemoveFlags(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
	}

	Msg("cvar_unhide_all: Removed FCVAR_DEVELOPMENTONLY and FCVAR_HIDDEN from %d ConVars\n", nUnhidden);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CON_COMMAND(find_all, "Replica of \"find\". Ignores FCVAR_HIDDEN or FCVAR_DEVELOPMENTONLY flags")
{
	if (args.ArgC() != 2)
	{
		Warning("find_full: Syntax: <search>\n");
		return;
	}

	ICvar::Iterator iter(g_pCVar);

	for (iter.SetFirst(); iter.IsValid(); iter.Next())
	{
		ConCommandBase *cmd = iter.Get();

		if (!V_stristr(cmd->GetName(), args[1]) && !V_stristr(cmd->GetHelpText(), args[1]))
			continue;

		ConVar_PrintDescription(cmd);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
static std::string ConvarFlagsString(const ConCommandBase *cmd)
{
	std::vector<std::string> flags;

	if (cmd->IsFlagSet(FCVAR_DEVELOPMENTONLY))
		flags.push_back("devonly");
	if (cmd->IsFlagSet(FCVAR_GAMEDLL))
		flags.push_back("sv");
	if (cmd->IsFlagSet(FCVAR_CLIENTDLL))
		flags.push_back("cl");
	if (cmd->IsFlagSet(FCVAR_HIDDEN))
		flags.push_back("hidden");
	if (cmd->IsFlagSet(FCVAR_PROTECTED))
		flags.push_back("prot");
	if (cmd->IsFlagSet(FCVAR_SPONLY))
		flags.push_back("sp");
	if (cmd->IsFlagSet(FCVAR_ARCHIVE))
		flags.push_back("a");
	if (cmd->IsFlagSet(FCVAR_NOTIFY))
		flags.push_back("nf");
	if (cmd->IsFlagSet(FCVAR_USERINFO))
		flags.push_back("user");
	if (cmd->IsFlagSet(FCVAR_PRINTABLEONLY))
		flags.push_back("print");
	if (cmd->IsFlagSet(FCVAR_NEVER_AS_STRING))
		flags.push_back("numeric");
	if (cmd->IsFlagSet(FCVAR_REPLICATED))
		flags.push_back("rep");
	if (cmd->IsFlagSet(FCVAR_CHEAT))
		flags.push_back("cheat");
	if (cmd->IsFlagSet(FCVAR_SS))
		flags.push_back("ss");
	if (cmd->IsFlagSet(FCVAR_DEMO))
		flags.push_back("demo");
	if (cmd->IsFlagSet(FCVAR_DONTRECORD))
		flags.push_back("norecord");

	std::string result;
	bool bFirst = true;

	for (auto flag : flags)
	{
		if (bFirst)
			bFirst = false;
		else
			result += ", ";

		result += "\"" + flag + "\"";
	}

	return result;
}

void string_replace(std::string& data, std::string toSearch, std::string replaceStr)
{
	// Get the first occurrence
	size_t pos = data.find(toSearch);

	// Repeat till end is reached
	while (pos != std::string::npos)
	{
		// Replace this occurrence of Sub String
		data.replace(pos, toSearch.size(), replaceStr);
		// Get the next occurrence from the current position
		pos = data.find(toSearch, pos + replaceStr.size());
	}
}

CON_COMMAND(cvarlist_all, "List all ConVars. Syntax: [hidden]")
{
	ICvar::Iterator iter(g_pCVar);

	std::map<std::string, ConCommandBase*> allCvars;

	for (iter.SetFirst(); iter.IsValid(); iter.Next())
	{
		ConCommandBase *cmd = iter.Get();

		if(args.FindArg("hidden") && !cmd->IsFlagSet(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN))
			continue;

		allCvars[cmd->GetName()] = cmd;
	}

	Msg("Name | Default | Flags | Description\n");
	Msg("---- | ------- | ----- | -----------\n");

	for (auto pair : allCvars)
	{
		auto name = pair.first;
		auto cmd = pair.second;

		Msg("%-41s | ", name.c_str());

		if (cmd->IsCommand())
		{
			Msg("%-8s | ", "cmd");
		}
		else
		{
			auto cvar = static_cast<ConVar*>(cmd);
			Msg("%-8s | ", cvar->GetDefault());
		}
		
		Msg("%-16s | ", ConvarFlagsString(cmd).c_str());

		std::string helpText{ cmd->GetHelpText() };

		string_replace(helpText, "\\", "\\\\");
		string_replace(helpText, "<", "&lt;");
		string_replace(helpText, ">", "&gt;");
		string_replace(helpText, "[", "\\[");
		string_replace(helpText, "]", "\\]");
		string_replace(helpText, "\n", "<br>");
		string_replace(helpText, "|", "\\|");
		
		Msg("%s\n", helpText.c_str());
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CON_COMMAND(force_dispatch, "Dispatch a command regardless of any hidden, cheat or developmentonly flags")
{
	if (args.ArgC() < 2)
	{
		Warning("force_dispatch: Syntax: <command> [arg 1] [arg 2] [...] [arg n]\n");
		return;
	}

	ConCommand *pCC = g_pCVar->FindCommand(args[1]);
	if (!pCC)
	{
		Warning("force_dispatch: Unable to find ConCommand %s\n", args[1]);
		return;
	}

	CCommand cmd;
	cmd.Tokenize(args.ArgS());
	pCC->Dispatch(cmd);

	Msg("force_dispatch: Dispatched %s (args: %s)\n", args[1], cmd.ArgS());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class DummyCopier : ConCommandBase
{
public:
	char* CopyString(const char* from)
	{
		return ConCommandBase::CopyString(from);
	}

} copier;

#ifdef CVAR_UNHIDE_GAME_L4D2
struct CVValue_t
{
	char*& m_pszString;
	int& m_StringLength;
	int& m_nValue;
	float& m_fValue;
};
#endif

class CCvar
{
public:
	static void ForceSetValue(ConVar *pVar, const char *pszNewValue, bool bNoCallback)
	{
#ifdef CVAR_UNHIDE_GAME_L4D2
		CVValue_t value = { pVar->m_pszString, pVar->m_StringLength, pVar->m_nValue, pVar->m_fValue };
#else
		ConVar::CVValue_t& value = pVar->GetRawValue();
#endif

		char* pszOldValue = (char*)stackalloc(value.m_StringLength);
		memcpy(pszOldValue, value.m_pszString, value.m_StringLength);
		float flOldValue = value.m_fValue;

		value.m_nValue = atoi(pszNewValue);
		value.m_fValue = atof(pszNewValue);

		int len = Q_strlen(pszNewValue) + 1;

		if (len > value.m_StringLength)
		{
			// Copies string using tier1 local new/delete operators
			// This leaks the old string, but we don't have tier1's delete[] to hand
			value.m_pszString = copier.CopyString(pszNewValue);
			value.m_StringLength = len;
		}
		else
		{
			memcpy(value.m_pszString, pszNewValue, len);
		}

		// Invoke any necessary callback function
		if (!bNoCallback)
		{
#ifdef CVAR_UNHIDE_GAME_L4D2
			if (pVar->m_fnChangeCallback != NULL)
			{
				(*pVar->m_fnChangeCallback)(pVar, pszOldValue, flOldValue);
			}
#else
			for (int i = 0; i < pVar->GetChangeCallbackCount(); i++)
			{
				pVar->GetChangeCallback(i)(pVar, pszOldValue, flOldValue);
			}
#endif

			g_pCVar->CallGlobalChangeCallbacks(pVar, pszOldValue, flOldValue);
		}
	}
};

CON_COMMAND(cvar_set, "Set the value of a ConVar regardless of its maximum/minimum values")
{
	if (args.ArgC() < 3)
	{
		Warning("cvar_set: Syntax: <var> <value> [nocallback]\n");
		return;
	}

	ConVar *pVar = g_pCVar->FindVar(args[1]);
	if (!pVar)
	{
		Warning("cvar_set: Unable to find ConVar %s\n", args[1]);
		return;
	}

	bool bNoCallback = args.FindArg("nocallback") != NULL;
	const char *pszNewValue = args[2];

	CCvar::ForceSetValue(pVar, pszNewValue, bNoCallback);
	Msg("cvar_set: Set %s => %s\n", args[1], pszNewValue);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void DumpSendTable(SendTable *pTable, int nDepth)
{
	if (!pTable || nDepth == 0)
		return;

	static int s_iIndent = 0;

	s_iIndent += 4;

	Msg("%*s Table: %s [%d props]\n", s_iIndent, "-", pTable->GetName(), pTable->GetNumProps());

	for (int i = 0; i < pTable->GetNumProps(); ++i)
	{
		SendProp *pProp = pTable->GetProp(i);

		s_iIndent += 4;

		Msg("%*s %s (offset: %d, type: ", s_iIndent, "-", pProp->GetName(), pProp->GetOffset());

		switch (pProp->GetType())
		{
		case DPT_Int: Msg("int"); break;
		case DPT_Float: Msg("float"); break;
		case DPT_Vector: Msg("vector"); break;
		case DPT_VectorXY: Msg("vector2d"); break;
		case DPT_String: Msg("string"); break;
		case DPT_Array: Msg("array[%d]", pProp->GetNumElements()); break;
		case DPT_DataTable: Msg("datatable"); break;
#ifndef CVAR_UNHIDE_GAME_L4D2
		case DPT_Int64: Msg("int64"); break;
#endif
		default: Warning("unknown"); break;
		}

		Msg(")\n");

		if (pProp->GetType() == DPT_DataTable)
			DumpSendTable(pProp->GetDataTable(), nDepth-1);

		s_iIndent -= 4;
	}

	s_iIndent -= 4;
}

//-----------------------------------------------------------------------------
// Purpose: Dump all network props.
//-----------------------------------------------------------------------------
CON_COMMAND(dump_netprops, "Dump all network props. Syntax: [table depth = 1]. A table depth of -1 indicates infinite depth.")
{
	int nDepth = 1;
	if (args.ArgC() == 2)
		nDepth = atoi(args.Arg(1));

	if (serverGameDll == NULL &&
		!LoadInterface<IServerGameDLL>(serverGameDll, g_gameServerFactory, "ServerGameDLL", 5))
	{
		return;
	}

	for (ServerClass *pServerClass = serverGameDll->GetAllServerClasses(); pServerClass; pServerClass = pServerClass->m_pNext)
	{
		Msg("Class: %s [%d]\n", pServerClass->GetName(), pServerClass->m_ClassID);
		DumpSendTable(pServerClass->m_pTable, nDepth);
		Msg("\n");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Dump HLTV director mod events.
//-----------------------------------------------------------------------------
CON_COMMAND(hltv_modevents, "Dump HLTV director mod events.")
{
	if (hltvdirector == NULL &&
		!LoadInterface<IHLTVDirector>(hltvdirector, g_gameServerFactory, "HLTVDirector", 1))
	{
		return;
	}

	Msg("HLTVDirector::GetModEvents:\n");
	for (const char** modevents = hltvdirector->GetModEvents(); *modevents != NULL; ++modevents)
	{
		const char* eventname = *modevents;
		Msg("- %s\n", eventname);
	}
}