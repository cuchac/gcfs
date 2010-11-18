#include "gcfs_controls.h"
#include "gcfs_config_values.h"
#include "gcfs_task.h"
#include "gcfs_config.h"
#include "gcfs_service.h"
#include "gcfs_utils.h"

#include <string.h>
#include <stdio.h>

std::string GCFS_Control::trimStr(const std::string& Src, const std::string& c)
{
    return GCFS_ConfigValue::trimStr(Src, c);
}

const char * GCFS_ControlStatus::statuses[] = {
    "new",
    "submitted",
    "running",
    "aborted",
    "failed",
    "finished",
    "suspended",
};

bool GCFS_ControlStatus::read(GCFS_Task* pTask, std::string &buff)
{
    int status = (int)g_sConfig.GetService(pTask->m_iService.m_iValue)->getTaskStatus(pTask);

    char cbuff[32];
    snprintf(cbuff, ARRAYSIZE(cbuff), "%d\t%s", status, statuses[status]);

    buff = cbuff;

    return true;
}

bool GCFS_ControlStatus::write(GCFS_Task* pTask, const char * sValue)
{
    return false;
}

GCFS_ControlControl::GCFS_ControlControl():GCFS_Control("control")
{
    m_vCommands.push_back("start");
    m_vCommands.push_back("start_and_wait");
    m_vCommands.push_back("wait");
    m_vCommands.push_back("abort");
    m_vCommands.push_back("suspend");
}

bool GCFS_ControlControl::read(GCFS_Task* pTask, std::string &buff)
{
    for (int iIndex = 0; iIndex < m_vCommands.size(); iIndex ++)
    {
        if (iIndex > 0)
            buff += ", ";

        buff += m_vCommands[iIndex];
    }

    return true;
}

bool GCFS_ControlControl::write(GCFS_Task* pTask, const char * sValue)
{
    std::string value = trimStr(sValue);

    int iVal = -1;
    for (int iIndex = 0; iIndex < m_vCommands.size(); iIndex ++)
        if (strcasecmp(m_vCommands[iIndex], value.c_str()) == 0)
            iVal = iIndex;

    if (iVal < 0)
        return false;

    GCFS_Service * pService = g_sConfig.GetService(pTask->m_iService.m_iValue);

    //Do propper action
    switch (iVal)
    {
    case eStart:
    {
        if (pTask->isSubmited())
            return false;

        if (!pTask->getExecutableFile())
            return false;

        return pService->submitTask(pTask);
        break;
    }
    case eStartAndWait:
    {
        if (this->write(pTask, "start"))
        {
            return pService->waitForTask(pTask);
        }
        else
            return false;
        break;
    }
    case eWait:
    {
        if (!pTask->isSubmited() || pTask->isFinished())
            return false;

        return pService->waitForTask(pTask);
        break;
    }
    case eAbort:
        break;
    case eSuspend:
        break;
    }

    return false;
}

