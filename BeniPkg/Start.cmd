::::
:: @Package     : BeniPkg
:: @FileName    : Start.cmd
:: @Date        : 20211002
:: @Author      : Jiangwei
:: @Version     : 1.0
:: @Description :
::   This is used to start QEMU using OVMF.fd/coreboot.rom as BIOS.
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

if exist OVMF*.fd (
    if exist disk.img (
        qemu-system-x86_64 -machine q35,smm=on -drive format=raw,file=disk.img -drive if=pflash,format=raw,unit=0,file=OVMF.fd -net nic -net tap,ifname=tap0 -serial stdio >> log.txt
        goto DONE
    )
    if /I "%1"=="NET" (
        qemu-system-x86_64 -machine q35,smm=on -drive if=pflash,format=raw,unit=0,file=OVMF.fd -net nic -net tap,ifname=tap0 -serial stdio >> log.txt
    ) else (
        qemu-system-x86_64 -machine q35,smm=on -drive if=pflash,format=raw,unit=0,file=OVMF.fd -serial stdio >> log.txt
    )
    goto DONE
)

if exist coreboot.rom (
    qemu-system-x86_64 -usb -bios coreboot.rom -serial stdio >> log.txt
    goto DONE
)

:DONE

echo on
@exit /b 0
