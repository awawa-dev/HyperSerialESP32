Import("env")
from pathlib import Path
import shutil


def post_program_action(source, target, env):
    program_path = target[0].get_abspath()
    path = Path(program_path)
    shutil.copy(program_path, path.parent.parent.absolute())
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", post_program_action)

env.Replace(PROGNAME="firmware_%s" % env.GetProjectOption("custom_prog_version"))

