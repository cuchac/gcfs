#include "gcfs_controls.h"
#include "gcfs_config_values.h"
#include "gcfs_task.h"
#include "gcfs_config.h"
#include "gcfs_service.h"

#include <string.h>

std::string GCFS_Control::trimStr(const std::string& Src, const std::string& c)
{
	return GCFS_ConfigValue::trimStr(Src, c);
}

bool GCFS_ControlStatus::read(GCFS_Task* pTask, std::string &buff)
{

}

bool GCFS_ControlStatus::write(GCFS_Task* pTask, const char * sValue)
{

}

GCFS_ControlControl::GCFS_ControlControl():GCFS_Control("control")
{
	m_vCommands.push_back("start");
	m_vCommands.push_back("start_and_wait");
	m_vCommands.push_back("abort");
	m_vCommands.push_back("suspend");
}

bool GCFS_ControlControl::read(GCFS_Task* pTask, std::string &buff)
{
	for(int iIndex = 0; iIndex < m_vCommands.size(); iIndex ++)
	{
		if(iIndex > 0)
			buff += ", ";

		buff += m_vCommands[iIndex];
	}

	return true;
}

bool GCFS_ControlControl::write(GCFS_Task* pTask, const char * sValue)
{
	std::string value = trimStr(sValue);

	int iVal = -1;
	for(int iIndex = 0; iIndex < m_vCommands.size(); iIndex ++)
		if(strcasecmp(m_vCommands[iIndex], value.c_str()) == 0)
			iVal = iIndex;

	if(iVal < 0)
		return false;

	//Do propper action
	switch(iVal)
	{
		case START:
			return g_sConfig.GetService(pTask->m_iService.m_iValue)->submitTask(pTask);
			break;
		case START_AND_WAIT:
			break;
		case ABORT:
			break;
		case SUSPEND:
			break;
	}
	
	return false;
}

