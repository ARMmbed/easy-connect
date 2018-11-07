""" ---
//----------------------------------------------------------------------------
//   The confidential and proprietary information contained in this file may
//   only be used by a person authorised under and to the extent permitted
//   by a subsisting licensing agreement from ARM Limited or its affiliates.
//
//          (C) COPYRIGHT 2013-2016 ARM Limited or its affiliates.
//              ALL RIGHTS RESERVED
//
//   This entire notice must be reproduced on all copies of this file
//   and copies of this file may only be made by a person if such person is
//   permitted to do so under the terms of a subsisting license agreement
//   from ARM Limited or its affiliates.
//----------------------------------------------------------------------------
--- """

# update *.lib in the root directory so they reference the latest revision of the repositories we need to use

import json
from sys import exit
import sys
import click
from os import path,getcwd,environ,remove
from shutil import copy2
from subprocess import check_output
import platform
import shutil

# update *.lib in the root directory so they reference the latest revision of the repositories we need to use


env_setup_file = 'env_setup.json'
# map QE lib file to the corresponding entry in the environment setup json file
num_of_arguments = len(sys.argv)

if(num_of_arguments > 1):
	operating_sys = sys.argv[1]
else:
	operating_sys = "linux"
	
if(operating_sys == "linux"):
	qe_lib_files = {
		'pal-platform.lib' 		   			: 'pal-platform',
		'sd-driver.lib'            			: 'sd-driver',
		'mbed-cloud-client.lib' 			: 'mbed-cloud-client-restricted',
		'easy-connect.lib'					: 'easy-connect',
		'e2eIoT-test-device.lib'   			: 'e2eIoT-test-device'
	}
else:
	qe_lib_files = {
		'mbed-os.lib'              			: 'mbed-os',
		'sd-driver.lib'            			: 'sd-driver',
		'mbed-cloud-client.lib' 			: 'mbed-cloud-client-restricted',
		'easy-connect.lib'					: 'easy-connect'
	}

class UpdateRepository(object):
    def __init__(self, vers):
        cwd = path.dirname(__file__)
        self.dlms_server_path = path.abspath(path.join(cwd, '../'))
        self.qe_root = path.abspath(path.join(cwd,'../'))
        self.ver = vers
        cache_path = path.abspath(path.join(cwd, '../../mbed_cache'))
        self.mbed_deploy_cmd = 'mbed new . --depth=1 && mbed config cache {} && mbed config protocol ssh && mbed config root . && mbed deploy'.format(cache_path)

    def write_lib_files(self):
        env_setup_path = path.join(self.dlms_server_path,env_setup_file)
        try:
            with open(env_setup_path) as f:
                self.repo_info = json.load(f)['repositories']
            click.echo("--------------------------")
            click.echo(" Writing .lib files:")
            click.echo("--------------------------")
            for lib_file in qe_lib_files.keys():
                click.echo(lib_file)
                dlms_ref = qe_lib_files[lib_file]
                lib_file_path = path.join(self.qe_root,lib_file)
                git_url = self.repo_info[dlms_ref]['git-url']
                # change git-url from ssh to https
                git_url = git_url.replace(':','/').replace('git@','https://').replace('.git','')
                repo_ref = '#'.join([git_url,self.repo_info[dlms_ref]['ref-spec']])
                with open(lib_file_path,'w') as lf:
                    lf.write(repo_ref)
            return True
        except Exception:
            return False

    def update_dependency_repositories(self):
        mbed_prog_folder = '.mbed'
        #click.echo('Creating new mbed program')
        if path.exists(mbed_prog_folder):
            remove(mbed_prog_folder)
        #if not self.sub_call('mbed new .'):
		#	return False
			
        if self.write_lib_files():
            click.echo("--------------------------------")
            click.echo("Updating dependency repositories")
            click.echo("--------------------------------")
            return self.sub_call(self.mbed_deploy_cmd)

    def replace_file(self, linker_uri, target_uri):
        try:
            shutil.copy(linker_uri, target_uri)
        except IOError, e:
            click.echo ("Unable to copy file. %s" % e)

    def sub_call(self,cmdline):
        cwd = self.dlms_server_path
        try:
            click.echo('check_output(cmdline="{}",cwd={})'.format(cmdline,cwd))
            if 'windows' in platform.platform().lower():
                out = check_output(cmdline, cwd=cwd)
            elif 'linux' in platform.platform().lower():
                out = check_output(cmdline, shell=True, cwd=cwd)
            else:
                out = "Unsupported operting system"
            click.echo(out)
            return True
        except Exception:
            return False


# @click.command()
# @click.option('--version',default='latest', type=click.Choice(['latest','release']))
def run():
    ver = 'latest'
    click.echo("selected version:{}".format(ver))
    u = UpdateRepository(ver)
    ok = False
    
    ok = u.update_dependency_repositories()
    
    if ok:
        click.echo ("Updating linker file")
        target_uri = path.join(getcwd(), 'mbed-os', 'targets', 'TARGET_Freescale', 'TARGET_MCUXpresso_MCUS',
                                  'TARGET_MCU_K64F', 'device', 'TOOLCHAIN_IAR', 'MK64FN1M0xxx12.icf')
    exit(0 if ok else 1)

if __name__ == '__main__':
    run()