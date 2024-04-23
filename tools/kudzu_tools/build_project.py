# Copyright 2023 The Pigweed Authors
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.
"""Implementation of `pw build` for this project."""

import logging
from pathlib import Path
import sys

from pw_build.build_recipe import (
    BuildCommand,
    BuildRecipe,
)
from pw_build.project_builder_presubmit_runner import (
    main,
    should_gn_gen_with_args,
)
from pw_presubmit.build import gn_args

_LOG = logging.getLogger('pw_build')


def build_project(force_pw_watch: bool = False) -> int:
    """Use pw_build to build this project.

    Defines BuildRecipes and passes them to Pigweed's
    project_builder_presubmit_runner.main to start a ``pw build`` invocation.

    Returns:
      An int representing the success or failure status of the build; 0 if
      successful, 1 if failed.

    Command line usage examples:

    .. code-block:: bash
       pw build --list
       pw build --recipe default
       pw build -C out --watch
    """

    default_gn_gen_command = [
        'gn',
        'gen',
        '{build_dir}',
        '--export-compile-commands',
    ]

    default_gn_args = dict(
        dir_pw_third_party_glfw="//environment/packages/glfw",
        dir_pw_third_party_imgui="//environment/packages/imgui",
    )

    default_gn_gen_command.append(gn_args(**default_gn_args))

    build_recipes = [
        BuildRecipe(
            build_dir=Path('out/gn'),
            title='default',
            steps=[
                BuildCommand(
                    run_if=should_gn_gen_with_args(default_gn_args),
                    command=default_gn_gen_command,
                ),
                BuildCommand(
                    build_system_command='ninja',
                    targets=['default'],
                ),
            ],
        ),
    ]

    # This project doesn't pass forward presubmit steps as they introduce some
    # clutter that is more likely to get in the way rather than help users.
    # If more comprehensive presubmit programs are added, they should be added
    # to `presubmit_programs` here.
    return main(
        build_recipes=build_recipes,
        default_build_recipe_names=['default'],
        default_root_logfile=Path('out/build.txt'),
        force_pw_watch=force_pw_watch,
    )


def watch_project() -> int:
    """Use ``pw watch`` to build this project."""
    sys.exit(build_project(force_pw_watch=True))


if __name__ == '__main__':
    sys.exit(build_project())
