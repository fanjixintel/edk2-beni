## @file
# Redfish DSC include file for Platform DSC
#
# This file includes all required information to enable Redfish features.
# It can be included to a platform DSC file by using "!include RedfishPkg/Redfish.dsc.inc".
#
# (C) Copyright 2020 Hewlett Packard Enterprise Development LP<BR>
#
#    SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[Defines]
  !include BeniPkg/BeniRedfishDefines.dsc.inc

[LibraryClasses]
  !include BeniPkg/BeniRedfishLibs.dsc.inc

[Components.X64]
  !include BeniPkg/BeniRedfishComponents.dsc.inc

