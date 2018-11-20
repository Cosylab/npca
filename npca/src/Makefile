TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#SHARED_LIBRARIES = YES
#STATIC_BUILD = NO

#=============================
# build an npca plugin

ifeq ($(OS_CLASS), WIN32)
LIBRARY += npca
else
LOADABLE_LIBRARY += npca
endif

npca_SRCS += npcashell.cpp plugin.cpp

# use embedded CA repeater
EMBED_CA_REPEATER = YES
ifeq ($(EMBED_CA_REPEATER), YES)
npca_SRCS += repeater.cpp
CPPFLAGS += -DEMBED_CA_REPEATER 
endif

# static flag
ifeq ($(STATIC_BUILD), YES)
CPPFLAGS += -DSTATIC_BUILD
endif

# specific OS support
npca_SRCS_Darwin += npmac.cpp
npca_SRCS_Linux += npunix.c
npca_SRCS_WIN32 += npwin.cpp

npca_LIBS += ca Com

USR_LDFLAGS = $(STATIC_LDFLAGS_$(STATIC_BUILD))

ifeq ($(OS_CLASS), WIN32)
GECKO_SDK = $(TOP)/gecko-sdk-win32
else
ifeq ($(OS_CLASS), Darwin)
GECKO_SDK = $(TOP)/gecko-sdk-mac
else
GECKO_SDK = $(TOP)/gecko-sdk
endif
endif

CPPFLAGS += -I$(GECKO_SDK)/include 


USR_CPPFLAGS_WIN32 += -DHAVE_MOZILLA_CONFIG_H 
npca_SYS_LIBS_WIN32 = ws2_32 advapi32 user32
npca_RCS_WIN32 = npca.rc

ifeq ($(OS_CLASS), WIN32)
# VS2005 needs this line when linking in a static build
LINK_OPT_FLAGS_YES += /MANIFEST 
endif

USR_CPPFLAGS_Darwin += -F/System/Library/Frameworks/CoreFoundation.framework \
	-I/Developer/Headers/FlatCarbon -DXP_MACOSX -DNO_X11 -DUSE_SYSTEM_CONSOLE
USR_LDFLAGS_Darwin += -framework Carbon


include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

ifeq ($(OS_CLASS), Darwin)

# additional build task, make plugin bundle
install: npca.plugin

npca.rsrc: ../macosx/npca.r
	@echo "Making resource file $@"
	$(CCC) -E -x c $< -include ../version.h | grep -v "^#" > $@.tmp
	/Developer/Tools/Rez -useDF /Developer/Headers/FlatCarbon/Types.r $@.tmp -o $@
	
Info.plist: ../macosx/Info.plist
	@$(CP) $< $@

npca.plugin: $(LOADABLE_SHRLIB_PREFIX)npca$(LOADABLE_SHRLIB_SUFFIX) npca.rsrc Info.plist
	@echo "Making Mac OS X plugin bundle $@"
	@$(CP) $(LOADABLE_SHRLIB_PREFIX)npca$(LOADABLE_SHRLIB_SUFFIX) npca
	@$(INSTALL) -d npca $(INSTALL_SHRLIB)/$@/Contents/MacOS
	@$(INSTALL) -d -m 644 npca.rsrc $(INSTALL_SHRLIB)/$@/Contents/Resources
	@$(INSTALL) -d -m 644 Info.plist $(INSTALL_SHRLIB)/$@/Contents
	@echo "... done."

endif


