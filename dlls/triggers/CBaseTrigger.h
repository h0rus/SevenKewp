#pragma once
#include "CBaseToggle.h"

#define SF_TRIGGER_HURT_TARGETONCE	1// Only fire hurt target once
#define	SF_TRIGGER_HURT_START_OFF	2//spawnflag that makes trigger_push spawn turned OFF
#define	SF_TRIGGER_HURT_NO_CLIENTS	8//spawnflag that makes trigger_push spawn turned OFF
#define SF_TRIGGER_HURT_CLIENTONLYFIRE	16// trigger hurt will only fire its target if it is hurting a client
#define SF_TRIGGER_HURT_CLIENTONLYTOUCH 32// only clients may touch this trigger.

#define SF_TRIGGER_FIRE_ON_ENTER 16
#define SF_TRIGGER_FIRE_ON_EXIT 32

// max number of entities that can be touching a "fire on enter/exit" trigger
#define MAX_TOUCHERS 64

class EXPORT CBaseTrigger : public CBaseToggle
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	void KeyValue(KeyValueData* pkvd);
	void MultiTouch(CBaseEntity* pOther);
	void HurtTouch(CBaseEntity* pOther);
	void ActivateMultiTrigger(CBaseEntity* pActivator, bool isUntouch);
	void CounterUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void InitTrigger(void);

	void UntouchThink();

	virtual int	ObjectCaps(void) { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	float m_nextTouch; // next time a touch is allowed
	EHANDLE m_touchers[MAX_TOUCHERS]; // toucher entindex
	int m_testTouch; // true touch method was called, for UntouchThink
};

