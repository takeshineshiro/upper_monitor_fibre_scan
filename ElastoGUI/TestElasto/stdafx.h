// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <assert.h>
#include <tchar.h>
#include <fstream>

#include "opencv/cv.h"
#include "opencv/highgui.h"


// TODO: reference additional headers your program requires here
// 算法代码的来源
#define	E_SOURCE_DLL     0      //来自dll代码
#define	E_SOURCE_GRAPHY  1      //来自Elastography代码
#define	E_SOURCE_SELF    2      //自己写的，现在主要来自Elastography

#define ELASTO_CODE      E_SOURCE_SELF