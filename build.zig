const std = @import("std");
const builtin = @import("builtin");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const LibraryType = enum { static, shared };
    const library_type = b.option(LibraryType, "libtype", "Build libui as a dynamically linked library") orelse .static;
    const is_dynamic = library_type == .shared;

    const lib = if (is_dynamic)
        b.addSharedLibrary(.{
            .name = "ui",
            .target = target,
            .optimize = optimize,
        })
    else
        b.addStaticLibrary(.{
            .name = "ui",
            .target = target,
            .optimize = optimize,
        });
    lib.linkLibC();
    lib.addIncludePath(.{ .path = "common" });
    lib.installHeader("ui.h", "ui.h");
    lib.defineCMacro("libui_EXPORTS", "");
    lib.addCSourceFiles(.{
        .files = &libui_common_sources,
        .flags = &.{},
    });

    if (target.result.isDarwin()) {
        // use darwin/*.m backend
        lib.installHeader("ui_darwin.h", "ui_darwin.h");
        lib.addIncludePath(.{ .path = "darwin" });
        lib.linkFramework("Foundation");
        lib.linkFramework("Appkit");
        lib.addSystemIncludePath(.{ .path = "Cocoa" });
        lib.addCSourceFiles(.{
            .files = &libui_darwin_sources,
            .flags = &.{},
        });
    } else if (target.result.os.tag == .windows) {
        // use windows/*.cpp backend
        lib.installHeader("ui_windows.h", "ui_windows.h");
        lib.subsystem = .Windows;
        lib.addIncludePath(.{ .path = "windows" });
        lib.linkSystemLibrary("user32");
        lib.linkSystemLibrary("kernel32");
        lib.linkSystemLibrary("gdi32");
        lib.linkSystemLibrary("comctl32");
        lib.linkSystemLibrary("uxtheme");
        lib.linkSystemLibrary("msimg32");
        lib.linkSystemLibrary("comdlg32");
        lib.linkSystemLibrary("d2d1");
        lib.linkSystemLibrary("dwrite");
        lib.linkSystemLibrary("ole32");
        lib.linkSystemLibrary("oleaut32");
        lib.linkSystemLibrary("oleacc");
        lib.linkSystemLibrary("uuid");
        lib.linkSystemLibrary("windowscodecs");
        lib.linkLibCpp();

        // Compile
        if (is_dynamic) {
            lib.addWin32ResourceFile(.{
                .file = .{ .path = "windows/resources.rc" },
                .flags = &.{},
            });
        }

        lib.addCSourceFiles(.{
            .files = &libui_windows_sources,
            .flags = if (is_dynamic) &.{} else &.{"-D_UI_STATIC"},
        });
    } else {
        // assume unix/*.c backend
        lib.installHeader("ui_unix.h", "ui_unix.h");
        lib.linkSystemLibrary("gtk+-3.0");
        lib.addIncludePath(.{ .path = "unix" });
        lib.addCSourceFiles(.{
            .files = &libui_unix_sources,
            .flags = &.{},
        });
    }

    b.installArtifact(lib);

    // Build examples
    const examples_step = b.step("examples", "Build all examples");
    const example_names = [_][]const u8{
        "controlgallery",
        "datetime",
        "drawtext",
        "hello-world",
        "histogram",
        "timer",
        "window",
    };
    inline for (example_names) |name| {
        const exe = b.addExecutable(.{
            .name = name,
            .target = target,
            .optimize = optimize,
        });
        exe.addCSourceFile(.{
            .file = .{ .path = "examples/" ++ name ++ "/main.c" },
            .flags = &.{},
        });
        exe.linkLibrary(lib);
        if (target.result.os.tag == .windows) {
            exe.addWin32ResourceFile(.{
                .file = .{ .path = "examples/resources.rc" },
                .flags = if (is_dynamic) &.{} else &.{ "/d", "_UI_STATIC" },
            });
        }
        const install_step = b.addInstallArtifact(exe, .{});
        const build_step = b.step("example-" ++ name, "Builds the " ++ name ++ " example");
        build_step.dependOn(&install_step.step);
        examples_step.dependOn(&install_step.step);

        const run = b.addRunArtifact(exe);
        const run_step = b.step("example-" ++ name ++ "-run", "Runs the " ++ name ++ " example");
        run_step.dependOn(&run.step);
    }

    // Build cpp-multithread example
    // Needs own build logic due to cpp
    {
        const exe = b.addExecutable(.{
            .name = "cpp-multithread",
            .target = target,
            .optimize = optimize,
        });
        exe.addCSourceFile(.{
            .file = .{ .path = "examples/cpp-multithread/main.cpp" },
            .flags = &.{},
        });
        exe.linkLibrary(lib);
        exe.linkLibCpp();

        if (target.result.os.tag == .windows) {
            exe.addWin32ResourceFile(.{
                .file = .{ .path = "examples/resources.rc" },
                .flags = if (is_dynamic) &.{} else &.{ "/d", "_UI_STATIC" },
            });
        }

        const install_step = b.addInstallArtifact(exe, .{});
        const build_step = b.step("example-cpp-multithread", "Builds the cpp-multithread example");
        build_step.dependOn(&install_step.step);
        examples_step.dependOn(&install_step.step);

        const run = b.addRunArtifact(exe);
        const run_step = b.step("example-cpp-multithread-run", "Runs the cpp-multithread example");
        run_step.dependOn(&run.step);
    }

    // Build test step
    const build_all_tests_step = b.step("tests", "Build all test executables (test, unit, qa)");
    const test_dir: std.Build.InstallDir = .{ .custom = "test" };
    {
        const exe = b.addExecutable(.{
            .name = "test",
            .target = target,
            .optimize = optimize,
            .win32_manifest = .{ .path = if (is_dynamic) "test/test.manifest" else "test/test.static.manifest" },
        });
        exe.addCSourceFiles(.{
            .files = &libui_test_sources,
            .flags = &.{},
        });
        exe.linkLibrary(lib);

        const install = b.addInstallArtifact(exe, .{
            .dest_dir = .{ .override = test_dir },
        });

        const tester = b.step("test", "Build the test executable");
        tester.dependOn(&install.step);

        build_all_tests_step.dependOn(&install.step);

        const run = b.addRunArtifact(exe);
        const run_step = b.step("test-run", "Runs the test executable");
        run_step.dependOn(&run.step);
    }

    // Run unit tests
    {
        const exe = b.addExecutable(.{
            .name = "unit",
            .target = target,
            .optimize = optimize,
            .win32_manifest = .{ .path = if (is_dynamic) "test/unit/unit.manifest" else "test/unit/unit.static.manifest" },
        });
        exe.addCSourceFiles(.{
            .files = &libui_unit_sources,
            .flags = &.{},
        });
        exe.addIncludePath(.{ .path = "test/unit/" });
        exe.linkLibrary(lib);
        exe.linkSystemLibrary("cmocka");

        const install = b.addInstallArtifact(exe, .{
            .dest_dir = .{ .override = test_dir },
        });

        const install_step = b.step("unit", "Build the unit test executable");
        install_step.dependOn(&install.step);

        build_all_tests_step.dependOn(&install.step);

        const run = b.addRunArtifact(exe);
        const run_step = b.step("unit-run", "Runs the unit test executable");
        run_step.dependOn(&run.step);
    }

    // Build qa binary
    {
        const exe = b.addExecutable(.{
            .name = "qa",
            .target = target,
            .optimize = optimize,
            .win32_manifest = .{ .path = if (is_dynamic) "test/qa/qa.manifest" else "test/qa/qa.static.manifest" },
        });
        exe.addCSourceFiles(.{
            .files = &libui_qa_sources,
            .flags = &.{},
        });
        exe.addIncludePath(.{ .path = "test/qa/" });
        exe.linkLibrary(lib);

        const install = b.addInstallArtifact(exe, .{
            .dest_dir = .{ .override = test_dir },
        });

        const install_step = b.step("qa", "Build the qa test executable");
        install_step.dependOn(&install.step);

        build_all_tests_step.dependOn(&install.step);

        const run = b.addRunArtifact(exe);
        const run_step = b.step("qa-run", "Runs the test executable");
        run_step.dependOn(&run.step);
    }
}

const libui_common_sources = [_][]const u8{
    "common/areaevents.c",
    "common/attribute.c",
    "common/attrlist.c",
    "common/attrstr.c",
    "common/control.c",
    "common/debug.c",
    "common/matrix.c",
    "common/opentype.c",
    "common/shouldquit.c",
    "common/table.c",
    "common/tablemodel.c",
    "common/tablevalue.c",
    "common/userbugs.c",
    "common/utf.c",
};

const libui_darwin_sources = [_][]const u8{
    "darwin/aat.m",
    "darwin/alloc.m",
    "darwin/areaevents.m",
    "darwin/area.m",
    "darwin/attrstr.m",
    "darwin/autolayout.m",
    "darwin/box.m",
    "darwin/button.m",
    "darwin/checkbox.m",
    "darwin/colorbutton.m",
    "darwin/combobox.m",
    "darwin/control.m",
    "darwin/datetimepicker.m",
    "darwin/debug.m",
    "darwin/draw.m",
    "darwin/drawtext.m",
    "darwin/editablecombo.m",
    "darwin/entry.m",
    "darwin/event.m",
    "darwin/fontbutton.m",
    "darwin/fontmatch.m",
    "darwin/fonttraits.m",
    "darwin/fontvariation.m",
    "darwin/form.m",
    "darwin/future.m",
    "darwin/graphemes.m",
    "darwin/grid.m",
    "darwin/group.m",
    "darwin/image.m",
    "darwin/label.m",
    "darwin/main.m",
    "darwin/menu.m",
    "darwin/multilineentry.m",
    "darwin/nstextfield.m",
    "darwin/opentype.m",
    "darwin/progressbar.m",
    "darwin/radiobuttons.m",
    "darwin/scrollview.m",
    "darwin/separator.m",
    "darwin/slider.m",
    "darwin/spinbox.m",
    "darwin/stddialogs.m",
    "darwin/tablecolumn.m",
    "darwin/table.m",
    "darwin/tab.m",
    "darwin/text.m",
    "darwin/undocumented.m",
    "darwin/util.m",
    "darwin/window.m",
    "darwin/winmoveresize.m",
};

const libui_windows_sources = [_][]const u8{
    "windows/alloc.cpp",
    "windows/area.cpp",
    "windows/areadraw.cpp",
    "windows/areaevents.cpp",
    "windows/areascroll.cpp",
    "windows/areautil.cpp",
    "windows/attrstr.cpp",
    "windows/box.cpp",
    "windows/button.cpp",
    "windows/checkbox.cpp",
    "windows/colorbutton.cpp",
    "windows/colordialog.cpp",
    "windows/combobox.cpp",
    "windows/container.cpp",
    "windows/control.cpp",
    "windows/d2dscratch.cpp",
    "windows/datetimepicker.cpp",
    "windows/debug.cpp",
    "windows/draw.cpp",
    "windows/drawmatrix.cpp",
    "windows/drawpath.cpp",
    "windows/drawtext.cpp",
    "windows/dwrite.cpp",
    "windows/editablecombo.cpp",
    "windows/entry.cpp",
    "windows/events.cpp",
    "windows/fontbutton.cpp",
    "windows/fontdialog.cpp",
    "windows/fontmatch.cpp",
    "windows/form.cpp",
    "windows/graphemes.cpp",
    "windows/grid.cpp",
    "windows/group.cpp",
    "windows/image.cpp",
    "windows/init.cpp",
    "windows/label.cpp",
    "windows/main.cpp",
    "windows/menu.cpp",
    "windows/multilineentry.cpp",
    "windows/opentype.cpp",
    "windows/parent.cpp",
    "windows/progressbar.cpp",
    "windows/radiobuttons.cpp",
    "windows/separator.cpp",
    "windows/sizing.cpp",
    "windows/slider.cpp",
    "windows/spinbox.cpp",
    "windows/stddialogs.cpp",
    "windows/tab.cpp",
    "windows/table.cpp",
    "windows/tabledispinfo.cpp",
    "windows/tabledraw.cpp",
    "windows/tableediting.cpp",
    "windows/tablemetrics.cpp",
    "windows/tabpage.cpp",
    "windows/text.cpp",
    "windows/utf16.cpp",
    "windows/utilwin.cpp",
    "windows/window.cpp",
    "windows/winpublic.cpp",
    "windows/winutil.cpp",
};

const libui_unix_sources = [_][]const u8{
    "unix/alloc.c",
    "unix/area.c",
    "unix/attrstr.c",
    "unix/box.c",
    "unix/button.c",
    "unix/cellrendererbutton.c",
    "unix/checkbox.c",
    "unix/child.c",
    "unix/colorbutton.c",
    "unix/combobox.c",
    "unix/control.c",
    "unix/datetimepicker.c",
    "unix/debug.c",
    "unix/draw.c",
    "unix/drawmatrix.c",
    "unix/drawpath.c",
    "unix/drawtext.c",
    "unix/editablecombo.c",
    "unix/entry.c",
    "unix/fontbutton.c",
    "unix/fontmatch.c",
    "unix/form.c",
    "unix/future.c",
    "unix/graphemes.c",
    "unix/grid.c",
    "unix/group.c",
    "unix/image.c",
    "unix/label.c",
    "unix/main.c",
    "unix/menu.c",
    "unix/multilineentry.c",
    "unix/opentype.c",
    "unix/progressbar.c",
    "unix/radiobuttons.c",
    "unix/separator.c",
    "unix/slider.c",
    "unix/spinbox.c",
    "unix/stddialogs.c",
    "unix/tab.c",
    "unix/table.c",
    "unix/tablemodel.c",
    "unix/text.c",
    "unix/util.c",
    "unix/window.c",
};

const libui_test_sources = [_][]const u8{
    "test/drawtests.c",
    "test/images.c",
    "test/main.c",
    "test/menus.c",
    "test/page1.c",
    "test/page2.c",
    "test/page3.c",
    "test/page4.c",
    "test/page5.c",
    "test/page6.c",
    "test/page7.c",
    "test/page7a.c",
    "test/page7b.c",
    "test/page7c.c",
    "test/page11.c",
    "test/page12.c",
    "test/page13.c",
    "test/page14.c",
    "test/page15.c",
    "test/page16.c",
    "test/page17.c",
    "test/spaced.c",
};

const libui_unit_sources = [_][]const u8{
    "test/unit/main.c",
    "test/unit/init.c",
    "test/unit/slider.c",
    "test/unit/spinbox.c",
    "test/unit/label.c",
    "test/unit/button.c",
    "test/unit/combobox.c",
    "test/unit/checkbox.c",
    "test/unit/drawmatrix.c",
    "test/unit/radiobuttons.c",
    "test/unit/entry.c",
    "test/unit/menu.c",
    "test/unit/progressbar.c",
};

const libui_qa_sources = [_][]const u8{
    "test/qa/qa.c",
    "test/qa/main.c",
    "test/qa/button.c",
    "test/qa/checkbox.c",
    "test/qa/entry.c",
    "test/qa/label.c",
    "test/qa/window.c",
};
