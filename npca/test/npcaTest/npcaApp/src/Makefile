TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# Build an IOC support library

LIBRARY_IOC += xxxSupport

# xxxRecord.h will be created from xxxRecord.dbd
DBDINC += xxxRecord
# install devXxxSoft.dbd into <top>/dbd
DBD += xxxSupport.dbd

# The following are compiled and added to the Support library
xxxSupport_SRCS += xxxRecord.c
xxxSupport_SRCS += devXxxSoft.c

xxxSupport_LIBS += $(EPICS_BASE_IOC_LIBS)

#=============================
# build an ioc application

PROD_IOC = npca
# npca.dbd will be created and installed
DBD += npca.dbd

# npca.dbd will be made up from these files:
npca_DBD += base.dbd
npca_DBD += xxxSupport.dbd
npca_DBD += dbSubExample.dbd
npca_DBD += npcaHello.dbd

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
npca_SRCS += npca_registerRecordDeviceDriver.cpp
npca_SRCS_DEFAULT += npcaMain.cpp
npca_SRCS_vxWorks += -nil-

# Add locally compiled object code
npca_SRCS += dbSubExample.c
npca_SRCS += npcaHello.c

# The following adds support from base/src/vxWorks
npca_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

npca_LIBS += xxxSupport

# NOTE: To build SNL programs, SNCSEQ must be defined
# in the <top>/configure/RELEASE file

ifneq ($(SNCSEQ),)
    # This builds sncExample as a component of npca
    npca_SNCFLAGS += +r
    npca_DBD += sncExample.dbd
    npca_SRCS += sncExample.stt
    npca_LIBS += seq pv

    # The following builds sncProgram as a standalone application
    PROD_HOST += sncProgram
    sncProgram_SNCFLAGS += +m
    sncProgram_SRCS += sncProgram.st
    sncProgram_LIBS += seq pv
    sncProgram_LIBS += $(EPICS_BASE_HOST_LIBS)
endif

npca_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

