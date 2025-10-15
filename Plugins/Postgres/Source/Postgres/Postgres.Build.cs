using UnrealBuildTool;
using System.IO;
using System;

public class Postgres : ModuleRules
{
    public Postgres(ReadOnlyTargetRules Target) : base(Target)
    {
        // Keep it simple (avoids Live Coding LNK2011 quirks for tiny modules)
        PCHUsage = PCHUsageMode.NoPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });

        string ThirdPartyPath = Path.Combine(ModuleDirectory, "../../ThirdParty/PostgreSQL");
        PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "include"));

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string LibPath = Path.Combine(ThirdPartyPath, "lib", "Win64");
            string BinPath = Path.Combine(ThirdPartyPath, "bin", "Win64");

            PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libpq.lib"));
            PublicDelayLoadDLLs.Add("libpq.dll"); // we’ll preload at runtime too

            // Where UBT normally puts editor runtime files
            string TargetOutDir = "$(TargetOutputDir)";

            // Also stage to the plugin's own Binaries/Win64 so the DLLs sit next to UnrealEditor-Postgres.dll
            string PluginBinDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Binaries/Win64"));
            Directory.CreateDirectory(PluginBinDir);

            // List all possible deps your libpq build might need (only staged if present)
            string[] Dlls =
            {
                "libpq.dll",

                // OpenSSL 3
                "libssl-3-x64.dll",
                "libcrypto-3-x64.dll",

                // OpenSSL 1.1  <-- add these
                // "libssl-1_1-x64.dll",
                // "libcrypto-1_1-x64.dll",

                // Optional
                "libiconv-2.dll",
                "libintl-9.dll",
                "zlib1.dll",
                "libzstd.dll",
                "liblz4.dll",
                "krb5_64.dll",
                "gssapi64.dll",
            };


            foreach (var dll in Dlls)
            {
                string src = Path.Combine(BinPath, dll);
                if (File.Exists(src))
                {
                    // Project binaries (what you already had)
                    RuntimeDependencies.Add($"{TargetOutDir}/{dll}", src);

                    // Plugin binaries (so they’re beside UnrealEditor-Postgres.dll)
                    RuntimeDependencies.Add(Path.Combine(PluginBinDir, dll), src);
                }
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string LibPath = Path.Combine(ThirdPartyPath, "lib", "Linux");
            string BinPath = Path.Combine(ThirdPartyPath, "bin", "Linux");

            PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libpq.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libpq.so", Path.Combine(BinPath, "libpq.so"));

            string[] Deps = { "libssl.so.3", "libcrypto.so.3" };
            foreach (var dep in Deps)
            {
                string src = Path.Combine(BinPath, dep);
                if (File.Exists(src))
                    RuntimeDependencies.Add("$(TargetOutputDir)/" + dep, src);
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            string LibPath = Path.Combine(ThirdPartyPath, "lib", "Mac");
            string BinPath = Path.Combine(ThirdPartyPath, "bin", "Mac");

            PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libpq.dylib"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libpq.dylib", Path.Combine(BinPath, "libpq.dylib"));

            string[] Deps = { "libssl.3.dylib", "libcrypto.3.dylib" };
            foreach (var dep in Deps)
            {
                string src = Path.Combine(BinPath, dep);
                if (File.Exists(src))
                    RuntimeDependencies.Add("$(TargetOutputDir)/" + dep, src);
            }
        }
    }
}
