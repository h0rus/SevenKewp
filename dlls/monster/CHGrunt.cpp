#include "extdll.h"
#include "plane.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "animation.h"
#include "CSquadMonster.h"
#include "weapons.h"
#include "CTalkMonster.h"
#include "env/CSoundEnt.h"
#include "effects.h"
#include "customentity.h"
#include "CBaseHGrunt.h"

#define	SENTENCE_VOLUME (float)0.35 // volume of grunt sentences

class CHGrunt : public CBaseHGrunt
{
public:
	void Spawn(void);
	void Precache(void);
	int Classify(void);
	void PainSound(void);
	void DeathSound(void);
	void IdleSound(void);
	void PlaySentenceSound(int sentenceType);

private:
	static const char* pPainSounds[];
	static const char* pDeathSounds[];
	static const char* pGruntSentences[];
};

LINK_ENTITY_TO_CLASS(monster_human_grunt, CHGrunt);

const char* CHGrunt::pPainSounds[] =
{
	"hgrunt/gr_pain1.wav",
	"hgrunt/gr_pain2.wav",
	"hgrunt/gr_pain3.wav",
	"hgrunt/gr_pain4.wav",
	"hgrunt/gr_pain5.wav"
};

const char* CHGrunt::pDeathSounds[] =
{
	"hgrunt/gr_die1.wav",
	"hgrunt/gr_pain2.wav",
	"hgrunt/gr_pain3.wav"
};

const char* CHGrunt::pGruntSentences[] =
{
	"HG_GREN", // grenade scared grunt
	"HG_ALERT", // sees player
	"HG_MONSTER", // sees monster
	"HG_COVER", // running to cover
	"HG_THROW", // about to throw grenade
	"HG_CHARGE",  // running out to get the enemy
	"HG_TAUNT", // say rude things
};

void CHGrunt::Spawn() {
	BaseSpawn("models/hgrunt.mdl");

	// get voice pitch
	if (RANDOM_LONG(0, 1))
		m_voicePitch = 109 + RANDOM_LONG(0, 7);
	else
		m_voicePitch = 100;
}

void CHGrunt::Precache()
{
	PRECACHE_MODEL("models/hgrunt.mdl");

	for (int i = 0; i < ARRAYSIZE(pPainSounds); i++)
		PRECACHE_SOUND((char*)pPainSounds[i]);

	for (int i = 0; i < ARRAYSIZE(pDeathSounds); i++)
		PRECACHE_SOUND((char*)pDeathSounds[i]);

	BasePrecache();
}

int	CHGrunt::Classify(void)
{
	return	CLASS_HUMAN_MILITARY;
}

void CHGrunt::PainSound(void)
{
	if (gpGlobals->time > m_flNextPainTime)
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, pPainSounds[RANDOM_LONG(0, ARRAYSIZE(pPainSounds) - 1)], 1, ATTN_NORM);
		m_flNextPainTime = gpGlobals->time + 1;
	}
}

void CHGrunt::DeathSound(void)
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, pDeathSounds[RANDOM_LONG(0, ARRAYSIZE(pDeathSounds) - 1)], 1, ATTN_NORM);
}

void CHGrunt::IdleSound(void)
{
	if (FOkToSpeak() && (g_fGruntQuestion || RANDOM_LONG(0, 1)))
	{
		if (!g_fGruntQuestion)
		{
			// ask question or make statement
			switch (RANDOM_LONG(0, 2))
			{
			case 0: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "HG_CHECK", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fGruntQuestion = 1;
				break;
			case 1: // question
				SENTENCEG_PlayRndSz(ENT(pev), "HG_QUEST", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fGruntQuestion = 2;
				break;
			case 2: // statement
				SENTENCEG_PlayRndSz(ENT(pev), "HG_IDLE", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
		}
		else
		{
			switch (g_fGruntQuestion)
			{
			case 1: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "HG_CLEAR", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			case 2: // question 
				SENTENCEG_PlayRndSz(ENT(pev), "HG_ANSWER", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
			g_fGruntQuestion = 0;
		}
		JustSpoke();
	}
}

void CHGrunt::PlaySentenceSound(int sentenceType) {
	SENTENCEG_PlayRndSz(ENT(pev), pGruntSentences[sentenceType], SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
}


//=========================================================
// CBaseHGruntRepel - when triggered, spawns a monster_human_grunt
// repelling down a line.
//=========================================================

class CHGruntRepel : public CBaseMonster
{
public:
	void Spawn(void);
	void Precache(void);
	void EXPORT RepelUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	int m_iSpriteTexture;	// Don't save, precache
};

LINK_ENTITY_TO_CLASS(monster_grunt_repel, CHGruntRepel);

void CHGruntRepel::Spawn(void)
{
	Precache();
	pev->solid = SOLID_NOT;

	SetUse(&CHGruntRepel::RepelUse);
}

void CHGruntRepel::Precache(void)
{
	UTIL_PrecacheOther("monster_human_grunt");
	m_iSpriteTexture = PRECACHE_MODEL("sprites/rope.spr");
}

void CHGruntRepel::RepelUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	TraceResult tr;
	UTIL_TraceLine(pev->origin, pev->origin + Vector(0, 0, -4096.0), dont_ignore_monsters, ENT(pev), &tr);
	/*
	if ( tr.pHit && Instance( tr.pHit )->pev->solid != SOLID_BSP)
		return NULL;
	*/

	CBaseEntity* pEntity = Create("monster_human_grunt", pev->origin, pev->angles);
	CBaseMonster* pGrunt = pEntity->MyMonsterPointer();
	pGrunt->pev->movetype = MOVETYPE_FLY;
	pGrunt->pev->velocity = Vector(0, 0, RANDOM_FLOAT(-196, -128));
	pGrunt->SetActivity(ACT_GLIDE);
	// UNDONE: position?
	pGrunt->m_vecLastPosition = tr.vecEndPos;

	CBeam* pBeam = CBeam::BeamCreate("sprites/rope.spr", 10);
	pBeam->PointEntInit(pev->origin + Vector(0, 0, 112), pGrunt->entindex());
	pBeam->SetFlags(BEAM_FSOLID);
	pBeam->SetColor(255, 255, 255);
	pBeam->SetThink(&CBeam::SUB_Remove);
	pBeam->pev->nextthink = gpGlobals->time + -4096.0 * tr.flFraction / pGrunt->pev->velocity.z + 0.5;

	UTIL_Remove(this);
}



//=========================================================
// DEAD HGRUNT PROP
//=========================================================
class CDeadHGrunt : public CBaseMonster
{
public:
	void Spawn(void);
	int	Classify(void) { return	CLASS_HUMAN_MILITARY; }

	void KeyValue(KeyValueData* pkvd);

	int	m_iPose;// which sequence to display	-- temporary, don't need to save
	static const char* m_szPoses[3];
};

const char* CDeadHGrunt::m_szPoses[] = { "deadstomach", "deadside", "deadsitting" };

void CDeadHGrunt::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseMonster::KeyValue(pkvd);
}

LINK_ENTITY_TO_CLASS(monster_hgrunt_dead, CDeadHGrunt);

void CDeadHGrunt::Spawn(void)
{
	PRECACHE_MODEL("models/hgrunt.mdl");
	SET_MODEL(ENT(pev), "models/hgrunt.mdl");

	pev->effects = 0;
	pev->yaw_speed = 8;
	pev->sequence = 0;
	m_bloodColor = BLOOD_COLOR_RED;

	pev->sequence = LookupSequence(m_szPoses[m_iPose]);

	if (pev->sequence == -1)
	{
		ALERT(at_console, "Dead hgrunt with bad pose\n");
	}

	// Corpses have less health
	pev->health = 8;

	// map old bodies onto new bodies
	switch (pev->body)
	{
	case 0: // Grunt with Gun
		pev->body = 0;
		pev->skin = 0;
		SetBodygroup(HEAD_GROUP, HEAD_GRUNT);
		SetBodygroup(GUN_GROUP, GUN_MP5);
		break;
	case 1: // Commander with Gun
		pev->body = 0;
		pev->skin = 0;
		SetBodygroup(HEAD_GROUP, HEAD_COMMANDER);
		SetBodygroup(GUN_GROUP, GUN_MP5);
		break;
	case 2: // Grunt no Gun
		pev->body = 0;
		pev->skin = 0;
		SetBodygroup(HEAD_GROUP, HEAD_GRUNT);
		SetBodygroup(GUN_GROUP, GUN_NONE);
		break;
	case 3: // Commander no Gun
		pev->body = 0;
		pev->skin = 0;
		SetBodygroup(HEAD_GROUP, HEAD_COMMANDER);
		SetBodygroup(GUN_GROUP, GUN_NONE);
		break;
	}

	MonsterInitDead();
}