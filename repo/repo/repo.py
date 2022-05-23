import os
from pathlib import Path
import re
import hubris
import hubris.repoman as repo
import hubris.platform as platform 

build_root = "_build"
install_root = "_install"

artifact_paths = [
	build_root,
	install_root,
	"_doxygen",
	"out",
]

deps = {
	"windows" : [
		"llvm"
	],
	"linux" : [
		"ninja-build",
		"clang"
	]
}


defs = []

defs.extend([
	# TODO : Improve this please.
	repo.CMakeDef("HTTPLIB_INSTALL", "OFF")
])

rman = repo.RepoMan()
rman.artifact_paths = artifact_paths

target_os = platform.get_os()
if target_os == platform.OS.windows:
	rman.deps = deps["windows"]
elif target_os == platform.OS.linux:
	rman.deps = deps["linux"]


def clean():
	result = rman.clean()
	if not result:
		hubris.log_error("Failed to clean repo")
	return result

def build():
	
	env = os.environ
	
	result = rman.build(
		build_root=build_root,
		source_root=".",
		defs=defs,
		env=env
	)
	if not result:
		hubris.log_error("Failed to build repo")
		return result

	result = rman.install(build_root=build_root)
	if not result:
		hubris.log_error("Failed to install repo")
		return result

	return result

def getdeps(force = False):
	result = rman.getdeps(force)
	if not result:
		hubris.log_error("Failed to get repo dependencies")
	if force:
		return True
	return result
