///////////////////////////////////////////////////////////////
// BastArtifact.cpp
// BastArtifact - �������� �������
///////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BastArtifact.h"
#include "PhysicsShell.h"
#include "extendedgeom.h"
#include "ParticlesObject.h"


CBastArtifact::CBastArtifact(void) 
{
	m_fImpulseThreshold = 10.f;
	
	m_fRadius = 10.f;
	m_fStrikeImpulse = 15.f;

	m_bStrike = false;
	m_AttakingEntity = NULL;

	m_fEnergy = 0.f;
	m_fEnergyMax = m_fStrikeImpulse * 100.f;
	m_fEnergyDecreasePerTime = 1.1f;

}

CBastArtifact::~CBastArtifact(void) 
{
}

//���������� ��� ������������ ������� � ���-��
void __stdcall CBastArtifact::ObjectContactCallback(bool& /**do_colide/**/,dContact& c,SGameMtl * /*material_1*/,SGameMtl * /*material_2*/) 
{
	dxGeomUserData *l_pUD1 = NULL;
	dxGeomUserData *l_pUD2 = NULL;
	l_pUD1 = retrieveGeomUserData(c.geom.g1);
	l_pUD2 = retrieveGeomUserData(c.geom.g2);

	if(!l_pUD1 || !l_pUD2) return;

	//���������� ��� ���� ���, �� ���� ������������� ���������
	CBastArtifact *pBastArtifact = l_pUD1 ? dynamic_cast<CBastArtifact*>(l_pUD1->ph_ref_object) : NULL;
	if(!pBastArtifact) pBastArtifact = l_pUD2 ? dynamic_cast<CBastArtifact*>(l_pUD2->ph_ref_object) : NULL;
	if(!pBastArtifact) return;
	if(!pBastArtifact->IsAttacking()) return;

	CEntityAlive *pEntityAlive = NULL;
	pEntityAlive = l_pUD1 ? dynamic_cast<CEntityAlive*>(l_pUD1->ph_ref_object) : NULL;
	if(!pEntityAlive) pEntityAlive = l_pUD2 ? dynamic_cast<CEntityAlive*>(l_pUD2->ph_ref_object) : NULL;

	pBastArtifact->BastCollision(pEntityAlive);
}

void CBastArtifact::BastCollision(CEntityAlive* pEntityAlive)
{
	//������ �� ���-�� �����
	if(pEntityAlive && pEntityAlive->g_Alive())
	{
		m_AttakingEntity = NULL;
		m_pHitedEntity = pEntityAlive;


		if(m_AliveList.size()>1)
		{
			m_bStrike = true;
		}
		else
		{
			m_bStrike = false;
		}

		m_bStrike = true;
		Fvector vel;
		vel.set(0,0,0);
	//	this->m_pPhysicsShell->set_LinearVel(vel);
	//	this->m_pPhysicsShell->set_AngularVel(vel);

	}
}

BOOL CBastArtifact::net_Spawn(LPVOID DC)
{
	BOOL result = inherited::net_Spawn(DC);
	if(!result) return FALSE;

	m_bStrike = false;
	m_AttakingEntity = NULL;
	m_pHitedEntity = NULL;
	m_AliveList.clear();

	return TRUE;
}

void CBastArtifact::net_Destroy		()
{
	inherited::net_Destroy();

	m_bStrike = false;
	m_AttakingEntity = NULL;
	m_pHitedEntity = NULL;
	m_AliveList.clear();
}

void CBastArtifact::Load(LPCSTR section) 
{
	// verify class
	LPCSTR Class = pSettings->r_string(section,"class");
	CLASS_ID load_cls = TEXT2CLSID(Class);
	R_ASSERT(load_cls==SUB_CLS_ID);

	inherited::Load(section);

	m_fImpulseThreshold = pSettings->r_float(section,"impulse_threshold");
	m_fRadius = pSettings->r_float(section,"radius");
	m_fStrikeImpulse = pSettings->r_float(section,"strike_impulse");
	
	m_fEnergyMax = pSettings->r_float(section,"energy_max");
	m_fEnergyDecreasePerTime = pSettings->r_float(section,"energy_decrease_speed");

	m_sParticleName = pSettings->r_string(section,"particle");

}

void CBastArtifact::shedule_Update(u32 dt) 
{
	inherited::shedule_Update(dt);

	Fvector	P; 
	P.set(Position());
	feel_touch_update(P,m_fRadius);
}


void CBastArtifact::UpdateCL() 
{
	//Log						("--- A - CBastArtifact",*cName());
	//Log						("--- A - CBastArtifact",renderable.xform);
	inherited::UpdateCL		();

	//���������� ������� �� ������� ���� �����������
	if(m_fEnergy>0) m_fEnergy -= m_fEnergyDecreasePerTime*Device.fTimeDelta;

	if (getVisible() && m_pPhysicsShell) {
		if(m_bStrike) {
			//������� ������, ���� ��� ��� �� �������
			if(!m_AliveList.empty() && m_AttakingEntity == NULL) {
				CEntityAlive* pEntityToHit = NULL;
				if(m_AliveList.size()>1)
				{
					do
					{
						int rnd = ::Random.randI(m_AliveList.size());
						pEntityToHit = m_AliveList[rnd];
					} while (pEntityToHit == m_pHitedEntity);
				}
				else
				{
					pEntityToHit = m_AliveList.front();
				}

				m_AttakingEntity = pEntityToHit;
			}
		}
		
		if(m_AttakingEntity)
		{
			if(m_AttakingEntity->g_Alive() && m_fEnergy>m_fStrikeImpulse)
			{
				m_fEnergy -= m_fStrikeImpulse;

				//������� �������� �� ��������� ����
				Fvector dir;
				m_AttakingEntity->Center(dir);
				dir.sub(this->Position()); 
				dir.y += ::Random.randF(-0.05f, 0.5f);
		
				m_pPhysicsShell->applyImpulse(dir, 
								  m_fStrikeImpulse * Device.fTimeDelta *
								  m_pPhysicsShell->getMass());
			}
			else
			{
				m_AttakingEntity = NULL;
				m_bStrike = false;
			}
		}



		if(m_fEnergy>0 && ::Random.randF(0.f, 1.0f)<(m_fEnergy/(m_fStrikeImpulse*100.f)))
		{
			CParticlesObject* pStaticPG;
			//pStaticPG = xr_new<CParticlesObject>("ghoul\\fx-01-camp-fire_00",Sector());
			pStaticPG = xr_new<CParticlesObject>(*m_sParticleName,Sector());
			//pStaticPG = xr_new<CParticlesObject>("weapons\\generic_shoot", Sector());
			Fmatrix pos; 
			pos.set(XFORM()); 
			Fvector vel; 
			//vel.sub(Position(),ps_Element(0).vPosition); 
			//vel.div((Level().timeServer()-ps_Element(0).dwTime)/1000.f);
			vel.set(0,0,0);
			pStaticPG->UpdateParent(pos, vel); 
			pStaticPG->Play();
		}

	} 
	else if(H_Parent()) XFORM().set(H_Parent()->XFORM());
}


void CBastArtifact::Hit(float P, Fvector &dir,	
						CObject* who, s16 element,
						Fvector position_in_object_space, 
						float impulse, 
						ALife::EHitType hit_type)
{
	if(impulse>m_fImpulseThreshold && !m_AliveList.empty())
	{
		m_bStrike = true;
		m_AttakingEntity = m_pHitedEntity = NULL;
		
		m_fEnergy += m_fStrikeImpulse*impulse;

		if(m_fEnergy>m_fEnergyMax) m_fEnergy = m_fEnergyMax;

		//���� ������� �� ������� �� ���������� ������ ���������
		impulse = 0;
	}
	
	inherited::Hit(P, dir, who, element, position_in_object_space, impulse, hit_type);
}


//������ ����� ������� ������ � ��������� ���������
bool CBastArtifact::Useful() const
{
	if(m_fEnergy>0) 
		return false;
	else 
		return true;

}

void CBastArtifact::feel_touch_new(CObject* O) 
{
	CEntityAlive* pEntityAlive = dynamic_cast<CEntityAlive*>(O);

	if(pEntityAlive && pEntityAlive->g_Alive()) 
	{
		m_AliveList.push_back(pEntityAlive);
	}
}

void CBastArtifact::feel_touch_delete(CObject* O) 
{
	CEntityAlive* pEntityAlive = dynamic_cast<CEntityAlive*>(O);

	if(pEntityAlive)
	{
			m_AliveList.erase(std::find(m_AliveList.begin(), 
										m_AliveList.end(), 
										pEntityAlive));
	}
}

BOOL CBastArtifact::feel_touch_contact(CObject* O) 
{
	CEntityAlive* pEntityAlive = dynamic_cast<CEntityAlive*>(O);

	if(pEntityAlive && pEntityAlive->g_Alive()) 
		return TRUE;
	else
		return FALSE;
}

void CBastArtifact::setup_physic_shell	()
{
	inherited::setup_physic_shell();
	m_pPhysicsShell->set_PhysicsRefObject(this);
	m_pPhysicsShell->set_ObjectContactCallback(ObjectContactCallback);
	m_pPhysicsShell->set_ContactCallback(NULL);
}