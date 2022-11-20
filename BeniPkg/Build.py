# coding=utf-8
#!/usr/bin/env python
##
# @Package     : BeniPkg
# @FileName    : Build.py
# @Date        : 20221120
# @Author      : Jiangwei
# @Version     : 1.0
# @Description :
#   This is used to build OVMF binary on both Windows and Linux.
#
# @History:
#   20221120: Initialize.
#
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution. The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
##

import os
import sys
import shutil
import subprocess
import argparse


def get_file_data(file, mode = 'rb'):
    '''
    Get data from file.
    '''
    return open(file, mode).read()


def run_process(arg_list, print_cmd = False, capture_out = False):
    '''
    Run command.
    '''
    sys.stdout.flush()
    if print_cmd:
        print(' '.join(arg_list))

    exc    = None
    result = 0
    output = ''
    try:
        if capture_out:
            output = subprocess.check_output(arg_list).decode()
        else:
            result = subprocess.call(arg_list)
    except Exception as ex:
        result = 1
        exc    = ex

    if result:
        if not print_cmd:
            print('Error in running process:\n  %s' % ' '.join(arg_list))
        if exc is None:
            sys.exit(1)
        else:
            raise exc

    return output


def get_visual_studio_info():
    '''
    Get Visual Studio information in Windows environment.
    '''
    toolchain        = ''
    toolchain_prefix = ''
    toolchain_path   = ''
    toolchain_ver    = ''

    # Check new Visual Studio Community version first.
    vswhere_path = "%s/Microsoft Visual Studio/Installer/vswhere.exe" % os.environ['ProgramFiles(x86)']
    if os.path.exists(vswhere_path):
        cmd = [vswhere_path, '-all', '-property', 'installationPath']
        lines = run_process(cmd, capture_out = True)
        vscommon_path = ''
        for each in lines.splitlines ():
            each = each.strip()
            if each and os.path.isdir(each):
                vscommon_path = each
        vcver_file = vscommon_path + '\\VC\\Auxiliary\\Build\\Microsoft.VCToolsVersion.default.txt'
        if os.path.exists(vcver_file):
            for vs_ver in ['2017', '2019']:
                check_path = '\\Microsoft Visual Studio\\%s\\' % vs_ver
                if check_path in vscommon_path:
                    toolchain_ver    = get_file_data(vcver_file, 'r').strip()
                    toolchain_prefix = 'VS%s_PREFIX' % (vs_ver)
                    toolchain_path   = vscommon_path + '\\VC\\Tools\\MSVC\\%s\\' % toolchain_ver
                    toolchain='VS%s' % (vs_ver)
                    break

    if toolchain == '':
        vs_ver_list = [
            ('2015', 'VS140COMNTOOLS'),
            ('2013', 'VS120COMNTOOLS')
        ]
        for vs_ver, vs_tool in vs_ver_list:
            if vs_tool in os.environ:
                toolchain        ='VS%s%s' % (vs_ver, 'x86')
                toolchain_prefix = 'VS%s_PREFIX' % (vs_ver)
                toolchain_path   = os.path.join(os.environ[vs_tool], '..//..//')
                toolchain_ver    = vs_ver
                parts   = os.environ[vs_tool].split('\\')
                vs_node = 'Microsoft Visual Studio '
                for part in parts:
                    if part.startswith(vs_node):
                        toolchain_ver = part[len(vs_node):]
                break

    return (toolchain, toolchain_prefix, toolchain_path, toolchain_ver)


def prep_env(work_dir):
    '''
    Prepare environment for BIOS building.
    '''
    os.chdir(work_dir)

    if os.name == 'posix':
        toolchain = 'GCC49'
        gcc_ver = subprocess.Popen(['gcc', '-dumpversion'], stdout=subprocess.PIPE)
        (gcc_ver, err) = subprocess.Popen(['sed', 's/\\..*//'], stdin=gcc_ver.stdout, stdout=subprocess.PIPE).communicate()
        if int(gcc_ver) > 4:
            toolchain = 'GCC5'

        os.environ['PATH'] = os.environ['PATH'] + ':' + os.path.join(work_dir, 'BaseTools/BinWrappers/PosixLike')
        toolchain_ver = gcc_ver
    elif os.name == 'nt':
        os.environ['PATH'] = os.environ['PATH'] + ';' + os.path.join(work_dir, 'BaseTools\\Bin\\Win32')
        os.environ['PATH'] = os.environ['PATH'] + ';' + os.path.join(work_dir, 'BaseTools\\BinWrappers\\WindowsLike')
        os.environ['PYTHONPATH'] = os.path.join(work_dir, 'BaseTools', 'Source', 'Python')

        toolchain, toolchain_prefix, toolchain_path, toolchain_ver = get_visual_studio_info ()
        if toolchain:
            os.environ[toolchain_prefix] = toolchain_path
        else:
            print("Could not find supported Visual Studio version !")
            sys.exit(1)
        if 'NASM_PREFIX' not in os.environ:
            os.environ['NASM_PREFIX'] = "C:\\Nasm\\"
        if 'OPENSSL_PATH' not in os.environ:
            os.environ['OPENSSL_PATH'] = "C:\\Openssl\\"
        if 'IASL_PREFIX' not in os.environ:
            os.environ['IASL_PREFIX'] = "C:\\ASL\\"
    else:
        print("Unsupported operating system!")
        sys.exit(1)

    print('Using %s, Version %s' % (toolchain, toolchain_ver))

    # Update Environment variables.
    os.environ['EDK_TOOLS_PATH'] = os.path.join(work_dir, 'BaseTools')
    os.environ['BASE_TOOLS_PATH'] = os.path.join(work_dir, 'BaseTools')
    if 'WORKSPACE' not in os.environ:
        os.environ['WORKSPACE'] = work_dir
    os.environ['CONF_PATH']     = os.path.join(os.environ['WORKSPACE'], 'Conf')
    os.environ['TOOL_CHAIN']    = toolchain

    return (work_dir, toolchain)


def check_files_exist(base_name_list, dir = '', ext = ''):
    '''
    Check if build tools exist.
    '''
    for each in base_name_list:
        if not os.path.exists(os.path.join (dir, each + ext)):
            return False
    return True


def rebuild_basetools():
    '''
    Build tools.
    '''
    exe_list = 'GenFfs GenFv GenFw GenSec LzmaCompress'.split()
    ret = 0
    workspace = os.environ['WORKSPACE']

    cmd = [sys.executable, '-c', 'import sys; import platform; print(", ".join([sys.executable, platform.python_version()]))']
    py_out = run_process(cmd, capture_out = True)
    parts  = py_out.split(',')
    if len(parts) > 1:
        py_exe, py_ver = parts
        os.environ['PYTHON_COMMAND'] = py_exe
        print('Using %s, Version %s' % (os.environ['PYTHON_COMMAND'], py_ver.rstrip()))
    else:
        os.environ['PYTHON_COMMAND'] = 'python'

    if os.name == 'posix':
        if not check_files_exist(exe_list, os.path.join(workspace, 'BaseTools', 'Source', 'C', 'bin')):
            ret = subprocess.call(['make', '-C', 'BaseTools'])
    elif os.name == 'nt':
        if not check_files_exist(exe_list, os.path.join(workspace, 'BaseTools', 'Bin', 'Win32'), '.exe'):
            print("Could not find pre-built BaseTools binaries, try to rebuild BaseTools ...")
            ret = run_process(['BaseTools\\toolsetup.bat', 'forcerebuild'])

    if ret:
        print("Build BaseTools failed, please check required build environment and utilities !")
        sys.exit(1)


def pre_build(args, target, toolchain, work_dir):
    '''
    Do somethinf before building.
    '''
    print('--- pre build start ---')

    rebuild_basetools()

    print('--- pre build end ---')


def do_build(args, target, toolchain):
    '''
    The building process.
    '''
    print('--- do build start---')

    if os.name == 'posix':
        build_str = 'build'
    else:
        build_str = 'build.bat'

    if args.emulator:
        pkg_str     = 'EmulatorPkg/EmulatorPkg.dsc'
        bootp_str   = '-a X64 -t %s -b %s -y Report.log' % (toolchain, target)
    elif args.payload:
        pkg_str     = 'UefiPayloadPkg/UefiPayloadPkg.dsc'
        bootp_str   = '-a IA32 -a X64 -t %s -b %s -D BOOTLOADER=SBL -y Report.log' % (toolchain, target)
    else:
        pkg_str     = 'BeniPkg/BeniPkg.dsc'
        bootp_str   = '-a IA32 -a X64 -t %s -b %s -y Report.log' % (toolchain, target)

    cmd = '%s -p %s %s' % (build_str, pkg_str, bootp_str)
    ret = subprocess.call(cmd.split(' '))
    if ret:
        print('Failed to build OVMF!')
        sys.exit(1)

    print('--- do build end ---')


def post_build(args, target, toolchain, work_dir):
    '''
    Do something after building.
    '''
    print('--- post build start ---')

    src_path = os.path.join(work_dir, 'Build\BeniPkg\DEBUG_%s\FV\OVMF.fd' % toolchain)
    dst_path = os.path.join(work_dir, 'BeniPkg')
    if os.path.exists(src_path):
        print('%s copied!' % src_path)
        shutil.copy (src_path, dst_path)

    print('--- post build end ---')


def main():
    '''
    Main process.
    '''
    curr_dir = os.path.dirname(os.path.realpath(__file__))
    work_dir = os.path.dirname(curr_dir)

    ap = argparse.ArgumentParser()
    sp = ap.add_subparsers(help='command')

    ######################################## BUILD ########################################
    def cmd_build(args):
        print('Building ...')

        if (args.release):
            target = 'RELEASE'
        else:
            target = 'DEBUG'

        workspace, toolchain = prep_env(work_dir)
        os.environ['WORKSPACE'] = workspace

        pre_build(args, target, toolchain, work_dir)
        do_build(args, target, toolchain)
        post_build(args, target, toolchain, work_dir)

    buildp = sp.add_parser('build', help='build BIOS binary')
    buildp.add_argument('-r',  '--release', action='store_true', help='Release build')
    buildp.add_argument('-e',  '--emulator', action='store_true', help='Build Windows emulator')
    buildp.add_argument('-p',  '--payload', action='store_true', help='Build payload')
    buildp.set_defaults(func=cmd_build)

    ######################################## CLEAN ########################################
    def cmd_clean(args):
        print('Cleaning ...')

        dir_list    = [
            'Build',
            'BuildFsp',
        ]
        file_list   = [
            'Report.log',
            'BeniPkg/log.txt',
            'BeniPkg/OVMF.fd',
            'Conf/.AutoGenIdFile.txt',
            'vc140.pdb',
        ]

        for dir in dir_list:
            dir_path = os.path.join(work_dir, dir)
            if os.path.exists(dir_path):
                print('Removing %s' % dir_path)
                shutil.rmtree(dir_path)
        for file in file_list:
            file_path = os.path.join(work_dir, file)
            if os.path.exists(file_path):
                print('Removing %s' % file_path)
                os.remove(file_path)

    cleanp = sp.add_parser('clean', help='clean build directory')
    cleanp.add_argument('-d', '--distclean', action='store_true', help='Distribution clean')
    cleanp.set_defaults(func=cmd_clean)

    ######################################## START ########################################
    def cmd_start(args):
        print('Starting BIOS ...')

        ovmf_bin    = os.path.join(curr_dir, 'OVMF.fd')
        disk_img    = os.path.join(curr_dir, 'disk.img')
        cmd         = 'qemu-system-x86_64 -machine q35,smm=on -drive if=pflash,format=raw,unit=0,file=OVMF.fd -serial stdio'

        if os.path.exists(disk_img):
            cmd = cmd + ' -drive format=raw,file=disk.img'
        if args.network:
            cmd = cmd + ' -net nic -net tap,ifname=tap0'

        if os.path.exists(ovmf_bin):
            ret = subprocess.call(cmd.split(' '))

    startp = sp.add_parser('start', help='start BIOS')
    startp.add_argument('-n', '--network', action='store_true', help='Add network support')
    startp.set_defaults(func=cmd_start)

    ########################################  END  ########################################
    args = ap.parse_args()
    if len(args.__dict__) <= 1:
        ap.print_help()
        ap.exit()
    args.func(args)


if __name__ == '__main__':
    sys.exit(main())
