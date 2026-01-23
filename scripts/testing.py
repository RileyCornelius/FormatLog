Import("env")

# Changes the src_dir to 'src' when running tests
# So that examples are not included in the build
# which leads to multiple definition errors (setup, loop)

# Check if 'test' command is being run
if "__test" in COMMAND_LINE_TARGETS:
    # Override src_dir for tests - use default 'src' directory
    env.Replace(PROJECT_SRC_DIR=env.subst("$PROJECT_DIR/src"))
