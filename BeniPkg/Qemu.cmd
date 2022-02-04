::::
:: @Package     : BeniPkg
:: @FileName    : Qemu.cmd
:: @Date        : 20211002
:: @Author      : Jiangwei
:: @Version     : 1.0
:: @Description :
::   This is used to start QEMU using OVMF.fd as BIOS.
::
:: @History:
::   20211002: Initialize.
::
:: This program and the accompanying materials
:: are licensed and made available under the terms and conditions of the BSD License
:: which accompanies this distribution. The full text of the license may be found at
:: http://opensource.org/licenses/bsd-license.php
::
:: THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
:: WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
::::

@echo off

if exist log.txt del log.txt

if exist OVMF.fd (
    if exist disk.img (
        @REM qemu-system-x86_64 -usb -bios OVMF.fd -serial stdio >> log.txt
        qemu-system-x86_64 -usb -bios OVMF.fd -serial stdio -drive format=raw,file=disk.img -net nic -net tap,ifname=tap0 >> log.txt
        goto DONE
    )
    qemu-system-x86_64 -usb -bios OVMF.fd -serial stdio >> log.txt
    ::@qemu-system-x86_64 -bios OVMF.fd -serial stdio -net nic -net tap,ifname=OpenVPN >> log.txt
    goto DONE
)

if exist coreboot.rom (
    qemu-system-x86_64 -usb -bios coreboot.rom -serial stdio >> log.txt
    ::@qemu-system-x86_64 -bios coreboot.rom -serial stdio -net nic -net tap,ifname=OpenVPN >> log.txt
    goto DONE
)

:DONE
echo on
@exit /b 0