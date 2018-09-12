#include "SequenceHeader.h"
#include "CommonDef.h"


ShManager::ShManager()
{

}

ShManager::~ShManager()
{

}

void ShManager::Init()
{
	//m_nMinVpsId = MAX_NUM_VPS;
	//m_nMinSpsId = MAX_NUM_SPS;
	//m_nMinPpsId = MAX_NUM_PPS;

	int		i;
	for (i = 0; i < 1; i++)
		m_ShBuffer[i] = NULL;
}

void ShManager::Destroy()
{
	int		i;
	for (i = 0; i < 1; i++)
	{
		SAFE_DELETES(m_ShBuffer[i]);
	}
}

void ShManager::storeSequenceHeader(SequenceHeader *seqHeader)
{
	if (m_ShBuffer[0])
	{
		SAFE_DELETES(m_ShBuffer[0]);
	}
	m_ShBuffer[0] = seqHeader;
	//if (vps->getVPSId() < m_nMinVpsId)
	//{
	//	m_nMinVpsId = vps->getVPSId();
	//}
};