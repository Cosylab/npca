#!../../bin/darwin-x86/npca

## You may have to change npca to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase("dbd/npca.dbd")
npca_registerRecordDeviceDriver(pdbbase)

## Load record instances
dbLoadRecords("db/test.db","")

asSetFilename("test.acf")

## Set this to see messages from mySub
#var mySubDebug 1

cd ${TOP}/iocBoot/${IOC}
iocInit()

## Start any sequence programs
#seq sncExample,"user=msekoranjaHost"
