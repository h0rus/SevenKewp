/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

Class Hierachy

CBaseEntity
	CBaseDelay
		CBaseToggle
			CBaseItem
			CBaseMonster
				CBaseCycler
				CBasePlayer
				CBaseGroup
*/
#pragma once

#define		MAX_PATH_SIZE	10 // max number of nodes available for a path.

// These are caps bits to indicate what an object's capabilities (currently used for save/restore and level transitions)
#define		FCAP_CUSTOMSAVE				0x00000001
#define		FCAP_ACROSS_TRANSITION		0x00000002		// should transfer between transitions
#define		FCAP_MUST_SPAWN				0x00000004		// Spawn after restore
#define		FCAP_DONT_SAVE				0x80000000		// Don't save this
#define		FCAP_IMPULSE_USE			0x00000008		// can be used by the player
#define		FCAP_CONTINUOUS_USE			0x00000010		// can be used by the player
#define		FCAP_ONOFF_USE				0x00000020		// can be used by the player
#define		FCAP_DIRECTIONAL_USE		0x00000040		// Player sends +/- 1 when using (currently only tracktrains)
#define		FCAP_MASTER					0x00000080		// Can be used to "master" other entities (like multisource)

// UNDONE: This will ignore transition volumes (trigger_transition), but not the PVS!!!
#define		FCAP_FORCE_TRANSITION		0x00000080		// ALWAYS goes across transitions

#include "archtypes.h"     // DAL
#include "saverestore.h"
#include "monster/schedule.h"

#ifndef MONSTEREVENT_H
#include "monster/monsterevent.h"
#endif

#include "Platform.h"
#include "CKeyValue.h"

// C functions for external declarations that call the appropriate C++ methods

extern "C" DLLEXPORT int GetEntityAPI( DLL_FUNCTIONS *pFunctionTable, int interfaceVersion );
extern "C" DLLEXPORT int GetEntityAPI2( DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion );

extern int DispatchSpawn( edict_t *pent );
extern void DispatchKeyValue( edict_t *pentKeyvalue, KeyValueData *pkvd );
extern void DispatchTouch( edict_t *pentTouched, edict_t *pentOther );
extern void DispatchUse( edict_t *pentUsed, edict_t *pentOther );
extern void DispatchThink( edict_t *pent );
extern void DispatchBlocked( edict_t *pentBlocked, edict_t *pentOther );
extern void DispatchSave( edict_t *pent, SAVERESTOREDATA *pSaveData );
extern int  DispatchRestore( edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity );
extern void	DispatchObjectCollsionBox( edict_t *pent );
extern void SaveWriteFields( SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount );
extern void SaveReadFields( SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount );
extern void SaveGlobalState( SAVERESTOREDATA *pSaveData );
extern void RestoreGlobalState( SAVERESTOREDATA *pSaveData );
extern void ResetGlobalState( void );
extern void SetObjectCollisionBox(entvars_t* pev);

typedef enum { USE_OFF = 0, USE_ON = 1, USE_SET = 2, USE_TOGGLE = 3 } USE_TYPE;

extern void FireTargets( const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

typedef void (CBaseEntity::*BASEPTR)(void);
typedef void (CBaseEntity::*ENTITYFUNCPTR)(CBaseEntity *pOther );
typedef void (CBaseEntity::*USEPTR)( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

// For CLASSIFY
#define	CLASS_NONE				0
#define CLASS_MACHINE			1
#define CLASS_PLAYER			2
#define	CLASS_HUMAN_PASSIVE		3
#define CLASS_HUMAN_MILITARY	4
#define CLASS_ALIEN_MILITARY	5
#define CLASS_ALIEN_PASSIVE		6
#define CLASS_ALIEN_MONSTER		7
#define CLASS_ALIEN_PREY		8
#define CLASS_ALIEN_PREDATOR	9
#define CLASS_INSECT			10
#define CLASS_PLAYER_ALLY		11
#define CLASS_PLAYER_BIOWEAPON	12 // hornets and snarks.launched by players
#define CLASS_ALIEN_BIOWEAPON	13 // hornets and snarks.launched by the alien menace
#define CLASS_ALIEN_RACE_X		14
#define CLASS_ALIEN_RACE_X_PITDRONE		15 // TODO: use this
#define CLASS_VEHICLE			16
#define	CLASS_BARNACLE			99 // special because no one pays attention to it, and it eats a wide cross-section of creatures.

class CBaseEntity;
class CBaseMonster;
class CBasePlayerWeapon;
class CTalkSquadMonster;
class CBaseToggle;


#define	SF_NORESPAWN	( 1 << 10 )// !!!set this bit on guns and stuff that should never respawn.

//
// EHANDLE. Safe way to point to CBaseEntities who may die between frames
//
class EXPORT EHANDLE
{
private:
	edict_t *m_pent;
	int		m_serialnumber;
public:
	edict_t *GetEdict( void );
	CBaseEntity *GetEntity( void );
	edict_t *Set( edict_t *pent );

	operator CBaseEntity *();

	CBaseEntity * operator = (CBaseEntity *pEntity);
	CBaseEntity * operator ->();
};

enum entindex_priority {
	// entity should have an index lower than 512 so that sprite attachments work
	// (skin is sent with 9 bits in delta.lst and this can't be changed for vanilla HL clients)
	ENTIDX_PRIORITY_HIGH,

	// entity is likely to be sent to clients (monsters, sprites, doors, etc.)
	ENTIDX_PRIORITY_NORMAL,

	// entity is server-side only and can have an index above the client limit
	// TODO: make a separate edict list for these, things like logic ents are basically a scripting engine
	ENTIDX_PRIORITY_LOW,
};

//
// Base Entity.  All entity types derive from this
//
class EXPORT CBaseEntity
{
public:
	// Constructor.  Set engine to use C/C++ callback functions
	// pointers to engine data
	entvars_t *pev;		// Don't need to save/restore this pointer, the engine resets it

	// path corners
	EHANDLE m_hGoalEnt;// path corner we are heading towards
	EHANDLE m_hLink;// used for temporary link-list operations. 

	// initialization functions
	virtual void	Spawn( void ) { return; }
	virtual void	Precache( void ) { return; }
	virtual void	KeyValue(KeyValueData* pkvd);
	virtual CKeyValue GetKeyValue(const char* keyName);
	CKeyValue GetCustomKeyValue(const char* keyName);
	std::map<std::string, CKeyValue>* GetCustomKeyValues();
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	virtual int		ObjectCaps( void ) { return FCAP_ACROSS_TRANSITION; }
	virtual void	Activate( void ) {}
	virtual int		GetEntindexPriority() { return ENTIDX_PRIORITY_NORMAL; }
	virtual bool	ForceVisChecks() { return false; } // return true for invisible ents that normally skip VIS checks in AddToFullPack
	virtual Vector	GetTargetOrigin() { return pev->origin; } // origin used for monster pathing and targetting

	// Setup the object->object collision box (pev->mins / pev->maxs is the object->world collision box)
	virtual void	SetObjectCollisionBox( void );

// Classify - returns the type of group (i.e, "houndeye", or "human military" so that monsters with different classnames
// still realize that they are teammates. (overridden for monsters that form groups)
	virtual int Classify ( void ) { return m_Classify; };
	virtual void DeathNotice ( entvars_t* pevChild) {}// monster maker children use this to tell the monster maker that they have died.


	static	TYPEDESCRIPTION m_SaveData[];

	virtual void	TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	virtual int		TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	virtual int		TakeHealth( float flHealth, int bitsDamageType, float healthcap=0);
	virtual void	Killed( entvars_t *pevAttacker, int iGib );
	virtual int		BloodColor( void ) { return DONT_BLEED; }
	virtual void	TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType );
	virtual BOOL    IsTriggered( CBaseEntity *pActivator ) {return TRUE;}
	virtual CBaseMonster *MyMonsterPointer( void ) { return NULL;}
	virtual CBasePlayerWeapon* GetWeaponPtr(void) { return NULL; };
	virtual CTalkSquadMonster * MyTalkSquadMonsterPointer( void ) { return NULL;}
	virtual CBaseToggle* MyTogglePointer(void) { return NULL; }
	virtual	int		GetToggleState( void ) { return TS_AT_TOP; }
	virtual void	AddPoints( int score, BOOL bAllowNegativeScore ) {}
	virtual void	AddPointsToTeam( int score, BOOL bAllowNegativeScore ) {}
	virtual BOOL	AddPlayerItem( CBasePlayerItem *pItem ) { return 0; }
	virtual BOOL	RemovePlayerItem( CBasePlayerItem *pItem ) { return 0; }
	virtual int 	GiveAmmo( int iAmount, const char *szName, int iMax ) { return -1; };
	virtual float	GetDelay( void ) { return 0; }
	virtual int		IsMoving( void ) { return pev->velocity != g_vecZero; }
	virtual void	OverrideReset( void ) {}
	virtual int		DamageDecal( int bitsDamageType );
	// This is ONLY used by the node graph to test movement through a door
	virtual void	SetToggleState( int state ) {}
	virtual void    StartSneaking( void ) {}
	virtual void    StopSneaking( void ) {}
	virtual BOOL	OnControls( entvars_t *otherPev ) { return FALSE; }
	virtual BOOL    IsSneaking( void ) { return FALSE; }
	virtual BOOL	IsAlive( void ) { return (pev->deadflag == DEAD_NO) && pev->health > 0; }
	virtual BOOL	IsBSPModel( void ) { return pev->solid == SOLID_BSP || pev->movetype == MOVETYPE_PUSHSTEP; }
	virtual BOOL	ReflectGauss( void ) { return ( IsBSPModel() && !pev->takedamage ); }
	virtual BOOL	HasTarget( string_t targetname ) { return FStrEq(STRING(targetname), STRING(pev->target) ); }
	virtual BOOL    IsInWorld( void );
	virtual	BOOL	IsMonster( void ) { return FALSE; }
	virtual	BOOL	IsPlayer( void ) { return FALSE; }
	virtual	BOOL	IsPlayerCorpse( void ) { return FALSE; }
	virtual BOOL	IsNetClient( void ) { return FALSE; }
	virtual BOOL	IsBreakable( void ) { return FALSE; }
	virtual BOOL	IsMachine(void) { return FALSE; };
	virtual const char *TeamID( void ) { return ""; }
	virtual const char* DisplayName() { return STRING(pev->classname); }
	virtual const char* GetDeathNoticeWeapon() { return STRING(pev->classname); };


//	virtual void	SetActivator( CBaseEntity *pActivator ) {}
	virtual CBaseEntity *GetNextTarget( void );
	
	// fundamental callbacks
	void (CBaseEntity ::*m_pfnThink)(void);
	void (CBaseEntity ::*m_pfnTouch)( CBaseEntity *pOther );
	void (CBaseEntity ::*m_pfnUse)( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void (CBaseEntity ::*m_pfnBlocked)( CBaseEntity *pOther );

	virtual void Think( void ) { if (m_pfnThink) (this->*m_pfnThink)(); };
	virtual void Touch( CBaseEntity *pOther ) { if (m_pfnTouch) (this->*m_pfnTouch)( pOther ); };
	virtual void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) 
	{ 
		if (m_pfnUse) 
			(this->*m_pfnUse)( pActivator, pCaller, useType, value );
	}
	virtual void Blocked( CBaseEntity *pOther ) { if (m_pfnBlocked) (this->*m_pfnBlocked)( pOther ); };

	// allow engine to allocate instance data
    void *operator new( size_t stAllocateBlock, entvars_t *pev )
	{
		return (void *)ALLOC_PRIVATE(ENT(pev), stAllocateBlock);
	};

	// don't use this.
#if _MSC_VER >= 1200 // only build this code if MSVC++ 6.0 or higher
	void operator delete(void *pMem, entvars_t *pev)
	{
		pev->flags |= FL_KILLME;
	};
#endif

	virtual void UpdateOnRemove( void );

	// common member functions
	void SUB_Remove( void );
	void SUB_DoNothing( void );
	void SUB_StartFadeOut ( void );
	void SUB_FadeOut ( void );
	void SUB_CallUseToggle( void ) { this->Use( this, this, USE_TOGGLE, 0 ); }
	int			ShouldToggle( USE_TYPE useType, BOOL currentState );
	void		FireBullets( ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting,	Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t *pevAttacker = NULL  );
	Vector		FireBulletsPlayer( ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting,	Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t *pevAttacker = NULL, int shared_rand = 0 );

	virtual CBaseEntity *Respawn( void ) { return NULL; }

	void SUB_UseTargets( CBaseEntity *pActivator, USE_TYPE useType, float value );
	// Do the bounding boxes of these two intersect?
	int		Intersects( CBaseEntity *pOther );
	void	MakeDormant( void );
	int		IsDormant( void );
	BOOL    IsLockedByMaster( void ) { return FALSE; }

	static CBaseEntity *Instance( edict_t *pent )
	{ 
		if ( !pent )
			pent = ENT(0);
		CBaseEntity *pEnt = (CBaseEntity *)GET_PRIVATE(pent); 
		return pEnt; 
	}

	static CBaseEntity *Instance( entvars_t *pev ) { return Instance( ENT( pev ) ); }
	static CBaseEntity *Instance( int eoffset) { return Instance( ENT( eoffset) ); }

	CBaseMonster *GetMonsterPointer( entvars_t *pevMonster ) 
	{ 
		CBaseEntity *pEntity = Instance( pevMonster );
		if ( pEntity )
			return pEntity->MyMonsterPointer();
		return NULL;
	}
	CBaseMonster *GetMonsterPointer( edict_t *pentMonster ) 
	{ 
		CBaseEntity *pEntity = Instance( pentMonster );
		if ( pEntity )
			return pEntity->MyMonsterPointer();
		return NULL;
	}


	// Ugly code to lookup all functions to make sure they are exported when set.
#ifdef _DEBUG
	void FunctionCheck( void *pFunction, char *name ) 
	{ 
		//if (pFunction && !NAME_FOR_FUNCTION((uint32)pFunction) )
		//	ALERT( at_error, "No EXPORT: %s:%s (%08lx)\n", STRING(pev->classname), name, (uint32)pFunction );
	}

	BASEPTR	ThinkSet( BASEPTR func, char *name ) 
	{ 
		m_pfnThink = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntity,m_pfnThink)))), name ); 
		return func;
	}
	ENTITYFUNCPTR TouchSet( ENTITYFUNCPTR func, char *name ) 
	{ 
		m_pfnTouch = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntity,m_pfnTouch)))), name ); 
		return func;
	}
	USEPTR	UseSet( USEPTR func, char *name ) 
	{ 
		m_pfnUse = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntity,m_pfnUse)))), name ); 
		return func;
	}
	ENTITYFUNCPTR	BlockedSet( ENTITYFUNCPTR func, char *name ) 
	{ 
		m_pfnBlocked = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntity,m_pfnBlocked)))), name ); 
		return func;
	}

#endif


	// virtual functions used by a few classes
	
	// used by monsters that are created by the MonsterMaker
	virtual	void UpdateOwner( void ) { return; };


	//
	static CBaseEntity *Create( const char *szName, const Vector &vecOrigin, const Vector &vecAngles, edict_t *pentOwner = NULL, std::map<std::string, std::string> keys= std::map<std::string, std::string>());

	virtual BOOL FBecomeProne( void ) {return FALSE;};
	edict_t *edict() { return ENT( pev ); };
	EOFFSET eoffset( ) { return OFFSET( pev ); };
	int	  entindex( ) { return ENTINDEX( edict() ); };

	virtual Vector Center( ) { return (pev->absmax + pev->absmin) * 0.5; }; // center point of entity
	virtual Vector EyePosition( ) { return pev->origin + pev->view_ofs; };			// position of eyes
	virtual Vector EarPosition( ) { return pev->origin + pev->view_ofs; };			// position of ears
	virtual Vector BodyTarget( const Vector &posSrc ) { return Center( ); };		// position to shoot at

	virtual int Illumination( ) { return GETENTITYILLUM( ENT( pev ) ); };

	virtual	BOOL FVisible ( CBaseEntity *pEntity );
	virtual	BOOL FVisible ( const Vector &vecOrigin );

	virtual void SetClassify(int iNewClassify);
	virtual int IRelationship(CBaseEntity* pTarget);
	static int IRelationship(int attackerClass, int victimClass);
	bool ShouldBlockFriendlyFire(entvars_t* attacker);

	// can the player using this entity physically touch the ent with their hand?
	// or is there something in the way? (player use code assumes arms have noclip)
	bool CanReach(CBaseEntity* toucher);

	//We use this variables to store each ammo count.
	int ammo_9mm;
	int ammo_357;
	int ammo_bolts;
	int ammo_buckshot;
	int ammo_rockets;
	int ammo_uranium;
	int ammo_hornets;
	int ammo_argrens;
	//Special stuff for grenades and satchels.
	float m_flStartThrow;
	float m_flReleaseThrow;
	int m_chargeReady;
	int m_fInAttack;

	enum EGON_FIRESTATE { FIRE_OFF, FIRE_CHARGE };
	int m_fireState;
	int	m_Classify;		// Classify, to let mappers override the default

	uint32_t m_audiblePlayers; // players in the audible set of this entity (invalid for invisible ents)
	uint32_t m_visiblePlayers; // players in the visible set of this entity (invalid for invisible ents)
};



// Ugly technique to override base member functions
// Normally it's illegal to cast a pointer to a member function of a derived class to a pointer to a 
// member function of a base class.  static_cast is a sleezy way around that problem.

#ifdef _DEBUG

#define SetThink( a ) ThinkSet( static_cast <void (CBaseEntity::*)(void)> (a), #a )
#define SetTouch( a ) TouchSet( static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a), #a )
#define SetUse( a ) UseSet( static_cast <void (CBaseEntity::*)(	CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )> (a), #a )
#define SetBlocked( a ) BlockedSet( static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a), #a )

#else

#define SetThink( a ) m_pfnThink = static_cast <void (CBaseEntity::*)(void)> (a)
#define SetTouch( a ) m_pfnTouch = static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a)
#define SetUse( a ) m_pfnUse = static_cast <void (CBaseEntity::*)( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )> (a)
#define SetBlocked( a ) m_pfnBlocked = static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a)

#endif


class EXPORT CPointEntity : public CBaseEntity
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	void	Spawn( void );
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
private:
};


typedef struct locksounds			// sounds that doors and buttons make when locked/unlocked
{
	string_t	sLockedSound;		// sound a door makes when it's locked
	string_t	sLockedSentence;	// sentence group played when door is locked
	string_t	sUnlockedSound;		// sound a door makes when it's unlocked
	string_t	sUnlockedSentence;	// sentence group played when door is unlocked

	int		iLockedSentence;		// which sentence in sentence group to play next
	int		iUnlockedSentence;		// which sentence in sentence group to play next

	float	flwaitSound;			// time delay between playing consecutive 'locked/unlocked' sounds
	float	flwaitSentence;			// time delay between playing consecutive sentences
	BYTE	bEOFLocked;				// true if hit end of list of locked sentences
	BYTE	bEOFUnlocked;			// true if hit end of list of unlocked sentences
} locksound_t;

void PlayLockSounds(entvars_t *pev, locksound_t *pls, int flocked, int fbutton);

//
// MultiSouce
//

#define MAX_MULTI_TARGETS	64 // maximum number of targets a single multi_manager entity may be assigned.
#define MS_MAX_TARGETS 32

class EXPORT CMultiSource : public CPointEntity
{
public:
	void Spawn( );
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int	ObjectCaps( void ) { return (CPointEntity::ObjectCaps() | FCAP_MASTER); }
	BOOL IsTriggered( CBaseEntity *pActivator );
	void Register( void );
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	EHANDLE		m_rgEntities[MS_MAX_TARGETS];
	int			m_rgTriggered[MS_MAX_TARGETS];

	int			m_iTotal;
	string_t	m_globalstate;
	bool m_registered;
};


//
// generic Delay entity.
//
class EXPORT CBaseDelay : public CBaseEntity
{
public:
	float		m_flDelay;
	int			m_iszKillTarget;

	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	virtual void	KeyValue( KeyValueData* pkvd);
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];
	// common member functions
	void SUB_UseTargets( CBaseEntity *pActivator, USE_TYPE useType, float value );
	void DelayThink( void );
};


class EXPORT CBaseAnimating : public CBaseDelay
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_NORMAL; }
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	// Basic Monster Animation functions
	float StudioFrameAdvance( float flInterval = 0.0 ); // accumulate animation frame time from last time called until now
	int	 GetSequenceFlags( void );
	virtual int  LookupActivity ( int activity );
	int  LookupActivityHeaviest ( int activity );
	int  LookupSequence ( const char *label );
	void ResetSequenceInfo ( );
	void DispatchAnimEvents ( float flFutureInterval = 0.1 ); // Handle events that have happend since last time called up until X seconds into the future
	virtual void HandleAnimEvent( MonsterEvent_t *pEvent ) { return; };
	float SetBoneController ( int iController, float flValue );
	void InitBoneControllers ( void );
	float SetBlending ( int iBlender, float flValue );
	void GetBonePosition ( int iBone, Vector &origin, Vector &angles );
	void GetAutomovement( Vector &origin, Vector &angles, float flInterval = 0.1 );
	int  FindTransition( int iEndingSequence, int iGoalSequence, int *piDir );
	void GetAttachment ( int iAttachment, Vector &origin, Vector &angles );
	int GetAttachmentCount();
	void SetBodygroup( int iGroup, int iValue );
	int GetBodygroup( int iGroup );
	int ExtractBbox( int sequence, float *mins, float *maxs );
	void SetSequenceBox( void );
	bool ActivityHasEvent(int activity, int event);

	// animation needs
	float				m_flFrameRate;		// computed FPS for current sequence
	float				m_flGroundSpeed;	// computed linear movement rate for current sequence
	float				m_flLastEventCheck;	// last time the event list was checked
	BOOL				m_fSequenceFinished;// flag set when StudioAdvanceFrame moves across a frame boundry
	BOOL				m_fSequenceLoops;	// true if the sequence loops
};


//
// generic Toggle entity.
//
#define	SF_ITEM_USE_ONLY	256 //  ITEM_USE_ONLY = BUTTON_USE_ONLY = DOOR_USE_ONLY!!! 

class EXPORT CBaseToggle : public CBaseAnimating
{
public:
	void				KeyValue( KeyValueData *pkvd );

	TOGGLE_STATE		m_toggle_state;
	float				m_flActivateFinished;//like attack_finished, but for doors
	float				m_flMoveDistance;// how far a door should slide or rotate
	float				m_flWait;
	float				m_flLip;
	float				m_flTWidth;// for plats
	float				m_flTLength;// for plats

	Vector				m_vecPosition1;
	Vector				m_vecPosition2;
	Vector				m_vecAngle1;
	Vector				m_vecAngle2;

	int					m_cTriggersLeft;		// trigger_counter only, # of activations remaining
	float				m_flHeight;
	EHANDLE				m_hActivator;
	void (CBaseToggle::*m_pfnCallWhenMoveDone)(void);
	Vector				m_vecFinalDest;
	Vector				m_vecFinalAngle;

	int					m_bitsDamageInflict;	// DMG_ damage type that the door or tigger does

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	virtual int		GetToggleState( void ) { return m_toggle_state; }
	virtual float	GetDelay( void ) { return m_flWait; }

	// common member functions
	void LinearMove( Vector	vecDest, float flSpeed );
	void LinearMoveDone( void );
	void AngularMove( Vector vecDestAngle, float flSpeed );
	void AngularMoveDone( void );
	BOOL IsLockedByMaster( void );

	virtual CBaseToggle* MyTogglePointer(void) { return this; }

	// monsters use this, but so could buttons for instance
	virtual void PlaySentence(const char* pszSentence, float duration, float volume, float attenuation);
	virtual void PlayScriptedSentence(const char* pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity* pListener);
	virtual void SentenceStop(void);
	virtual BOOL IsAllowedToSpeak() { return FALSE; }

	static float		AxisValue( int flags, const Vector &angles );
	static void			AxisDir( entvars_t *pev );
	static float		AxisDelta( int flags, const Vector &angle1, const Vector &angle2 );

	string_t m_sMaster;		// If this button has a master switch, this is the targetname.
							// A master switch must be of the multisource type. If all 
							// of the switches in the multisource have been triggered, then
							// the button will be allowed to operate. Otherwise, it will be
							// deactivated.
};
#define SetMoveDone( a ) m_pfnCallWhenMoveDone = static_cast <void (CBaseToggle::*)(void)> (a)


// people gib if their health is <= this at the time of death
#define	GIB_HEALTH_VALUE	-30

#define	ROUTE_SIZE			8 // how many waypoints a monster can store at one time
#define MAX_OLD_ENEMIES		4 // how many old enemies to remember

#define	bits_CAP_DUCK			( 1 << 0 )// crouch
#define	bits_CAP_JUMP			( 1 << 1 )// jump/leap
#define bits_CAP_STRAFE			( 1 << 2 )// strafe ( walk/run sideways)
#define bits_CAP_SQUAD			( 1 << 3 )// can form squads
#define	bits_CAP_SWIM			( 1 << 4 )// proficiently navigate in water
#define bits_CAP_CLIMB			( 1 << 5 )// climb ladders/ropes
#define bits_CAP_USE			( 1 << 6 )// open doors/push buttons/pull levers
#define bits_CAP_HEAR			( 1 << 7 )// can hear forced sounds
#define bits_CAP_AUTO_DOORS		( 1 << 8 )// can trigger auto doors
#define bits_CAP_OPEN_DOORS		( 1 << 9 )// can open manual doors
#define bits_CAP_TURN_HEAD		( 1 << 10)// can turn head, always bone controller 0

#define bits_CAP_RANGE_ATTACK1	( 1 << 11)// can do a range attack 1
#define bits_CAP_RANGE_ATTACK2	( 1 << 12)// can do a range attack 2
#define bits_CAP_MELEE_ATTACK1	( 1 << 13)// can do a melee attack 1
#define bits_CAP_MELEE_ATTACK2	( 1 << 14)// can do a melee attack 2

#define bits_CAP_FLY			( 1 << 15)// can fly, move all around

#define bits_CAP_DOORS_GROUP    (bits_CAP_USE | bits_CAP_AUTO_DOORS | bits_CAP_OPEN_DOORS)

// used by suit voice to indicate damage sustained and repaired type to player

// instant damage

#define DMG_GENERIC			0			// generic damage was done
#define DMG_CRUSH			(1 << 0)	// crushed by falling or moving object
#define DMG_BULLET			(1 << 1)	// shot
#define DMG_SLASH			(1 << 2)	// cut, clawed, stabbed
#define DMG_BURN			(1 << 3)	// heat burned
#define DMG_FREEZE			(1 << 4)	// frozen
#define DMG_FALL			(1 << 5)	// fell too far
#define DMG_BLAST			(1 << 6)	// explosive blast damage
#define DMG_CLUB			(1 << 7)	// crowbar, punch, headbutt
#define DMG_SHOCK			(1 << 8)	// electric shock
#define DMG_SONIC			(1 << 9)	// sound pulse shockwave
#define DMG_ENERGYBEAM		(1 << 10)	// laser or other high energy beam 
#define DMG_NEVERGIB		(1 << 12)	// with this bit OR'd in, no damage type will be able to gib victims upon death
#define DMG_ALWAYSGIB		(1 << 13)	// with this bit OR'd in, any damage type can be made to gib victims upon death.
#define DMG_DROWN			(1 << 14)	// Drowning
// time-based damage
#define DMG_TIMEBASED		(~(0x3fff))	// mask for time-based damage

#define DMG_PARALYZE		(1 << 15)	// slows affected creature down
#define DMG_NERVEGAS		(1 << 16)	// nerve toxins, very bad
#define DMG_POISON			(1 << 17)	// blood poisioning
#define DMG_RADIATION		(1 << 18)	// radiation exposure
#define DMG_DROWNRECOVER	(1 << 19)	// drowning recovery
#define DMG_ACID			(1 << 20)	// toxic chemicals or acid burns
#define DMG_SLOWBURN		(1 << 21)	// in an oven
#define DMG_SLOWFREEZE		(1 << 22)	// in a subzero freezer
#define DMG_MORTAR			(1 << 23)	// Hit by air raid (done to distinguish grenade from mortar)

// sven-specific damages
#define DMG_SNIPER			(1 << 24)	// hit by a Sniper
#define DMG_MEDKITHEAL		(1 << 25)	// medkit healing
#define DMG_LAUNCH			(1 << 26)	// launch into air
#define DMG_SHOCK_GLOW		(1 << 27)	// shock glow effect

// these are the damage types that are allowed to gib corpses
#define DMG_GIB_CORPSE		( DMG_CRUSH | DMG_FALL | DMG_BLAST | DMG_SONIC | DMG_CLUB )

// these are the damage types that have client hud art
#define DMG_SHOWNHUD		(DMG_POISON | DMG_ACID | DMG_FREEZE | DMG_SLOWFREEZE | DMG_DROWN | DMG_BURN | DMG_SLOWBURN | DMG_NERVEGAS | DMG_RADIATION | DMG_SHOCK)

// damage types that draw blood from the victim
#define DMG_BLOOD (DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB | DMG_ACID | DMG_MORTAR | DMG_SNIPER | DMG_ENERGYBEAM)

// NOTE: tweak these values based on gameplay feedback:

#define PARALYZE_DURATION	2		// number of 2 second intervals to take damage
#define PARALYZE_DAMAGE		1.0		// damage to take each 2 second interval

#define NERVEGAS_DURATION	2
#define NERVEGAS_DAMAGE		5.0

#define POISON_DURATION		5
#define POISON_DAMAGE		2.0

#define RADIATION_DURATION	2
#define RADIATION_DAMAGE	1.0

#define ACID_DURATION		2
#define ACID_DAMAGE			5.0

#define SLOWBURN_DURATION	2
#define SLOWBURN_DAMAGE		1.0

#define SLOWFREEZE_DURATION	2
#define SLOWFREEZE_DAMAGE	1.0


#define	itbd_Paralyze		0		
#define	itbd_NerveGas		1
#define	itbd_Poison			2
#define	itbd_Radiation		3
#define	itbd_DrownRecover	4
#define	itbd_Acid			5
#define	itbd_SlowBurn		6
#define	itbd_SlowFreeze		7
#define CDMG_TIMEBASED		8

// when calling KILLED(), a value that governs gib behavior is expected to be 
// one of these three values
#define GIB_NORMAL			0// gib if entity was overkilled
#define GIB_NEVER			1// never gib, no matter how much death damage is done ( freezing, etc )
#define GIB_ALWAYS			2// always gib ( Houndeye Shock, Barnacle Bite )

#define WATERLEVEL_DRY 0
#define WATERLEVEL_FEET 1
#define WATERLEVEL_WAIST 2
#define WATERLEVEL_HEAD 3

#define SF_ITEM_TOUCH_ONLY 128 // Pick this item up only by touching it.
#define SF_ITEM_USE_ONLY 256 // Pick this item up only by using it ('USE' key).
#define SF_ITEM_USE_WITHOUT_LOS 512 // Player can pick up this item even when it's not within his line of sight.

class CBaseMonster;
class CCineMonster;
class CSound;



//
// Weapons 
//

#define	BAD_WEAPON 0x00007FFF

//
// Converts a entvars_t * to a class pointer
// It will allocate the class and entity if necessary
//
template <class T> T * GetClassPtr( T *a )
{
	entvars_t *pev = (entvars_t *)a;

	// allocate entity if necessary
	bool allocate = pev == NULL;
	if (allocate)
		pev = VARS(CREATE_ENTITY());

	// get the private data
	a = (T *)GET_PRIVATE(ENT(pev));

	if (a == NULL) 
	{
		// allocate private data 
		a = new(pev) T; // TODO: shouldn't this use malloc? What does the engine do?
		a->pev = pev;
	}

	// only relocate if a new entity was allocated, else EHANDLEs will break
	return allocate ? (T*)RelocateEntIdx(a) : a;
}


/*
bit_PUSHBRUSH_DATA | bit_TOGGLE_DATA
bit_MONSTER_DATA
bit_DELAY_DATA
bit_TOGGLE_DATA | bit_DELAY_DATA | bit_MONSTER_DATA
bit_PLAYER_DATA | bit_MONSTER_DATA
bit_MONSTER_DATA | CYCLER_DATA
bit_LIGHT_DATA
path_corner_data
bit_MONSTER_DATA | wildcard_data
bit_MONSTER_DATA | bit_GROUP_DATA
boid_flock_data
boid_data
CYCLER_DATA
bit_ITEM_DATA
bit_ITEM_DATA | func_hud_data
bit_TOGGLE_DATA | bit_ITEM_DATA
EOFFSET
env_sound_data
env_sound_data
push_trigger_data
*/

#define TRACER_FREQ		4			// Tracers fire every 4 bullets

typedef struct _SelAmmo
{
	BYTE	Ammo1Type;
	BYTE	Ammo1;
	BYTE	Ammo2Type;
	BYTE	Ammo2;
} SelAmmo;


// this moved here from world.cpp, to allow classes to be derived from it
//=======================
// CWorld
//
// This spawns first when each level begins.
//=======================
class EXPORT CWorld : public CBaseEntity
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_HIGH; }
	void Spawn( void );
	void Precache( void );
	void KeyValue( KeyValueData *pkvd );
	void loadReplacementFiles();
	const char* getDisplayName() { return "World"; }

	string_t m_globalModelList;
	string_t m_globalSoundList;
	bool m_freeRoam;
};
