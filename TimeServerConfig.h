#pragma once
#include "resource.h"

class CTimeServerConfigApp : public CWinApp {
public:
    CTimeServerConfigApp();
public:
    virtual BOOL InitInstance();
    DECLARE_MESSAGE_MAP()
};