using UnrealBuildTool;
using System.IO;

public class Postgres : ModuleRules
{
    public Postgres(ReadOnlyTargetRules target) : base(target)
    {
        // Keep it simple (avoids Live Coding LNK2011 quirks for tiny modules)
        PCHUsage = PCHUsageMode.NoPCHs;

        PublicDependencyModuleNames.AddRange(["Core", "CoreUObject", "Engine", "Projects"] );
        PrivateDependencyModuleNames.AddRange(["Core", "CoreUObject", "Engine", "Projects"]);

        var thirdPartyPath = Path.Combine(ModuleDirectory, "../../ThirdParty/PostgreSQL");
        PublicIncludePaths.Add(Path.Combine(thirdPartyPath, "include"));

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            var libPath = Path.Combine(thirdPartyPath, "lib", "Win64");
            var binPath = Path.Combine(thirdPartyPath, "bin", "Win64");

            PublicAdditionalLibraries.Add(Path.Combine(libPath, "libpq.lib"));
            PublicDelayLoadDLLs.Add("libpq.dll"); // we’ll preload at runtime too

            // Where UBT normally puts editor runtime files
            const string targetOutDir = "$(TargetOutputDir)";

            // Also stage to the plugin's own Binaries/Win64 so the DLLs sit next to UnrealEditor-Postgres.dll
            var pluginBinDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Binaries/Win64"));
            Directory.CreateDirectory(pluginBinDir);

            // List all possible deps your libpq build might need (only staged if present)
            string[] dlls =
            [
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
                "libwinpthread-1.dll",
            ];


            foreach (var dll in dlls)
            {
                var src = Path.Combine(binPath, dll);
                if (File.Exists(src))
                {
                    // Project binaries (what you already had)
                    RuntimeDependencies.Add($"{targetOutDir}/{dll}", src);

                    // Plugin binaries (so they’re beside UnrealEditor-Postgres.dll)
                    RuntimeDependencies.Add(Path.Combine(pluginBinDir, dll), src);
                }
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			var libPath = Path.Combine(thirdPartyPath, "lib", "Linux");
			var binPath = Path.Combine(thirdPartyPath, "bin", "Linux");

			const string soName = "libpq.so";
			var pqFromLib = Path.Combine(libPath, soName);
			var pqFromBin = Path.Combine(binPath, soName);
			var pqSrc = File.Exists(pqFromLib) ? pqFromLib : pqFromBin;

			if (!File.Exists(pqSrc))
			{
				throw new BuildException($"libpq not found. Checked: {pqFromLib} and {pqFromBin}");
			}

			// Link against the actual file we found
			PublicAdditionalLibraries.Add(pqSrc);

			// Stage that same file next to the target binary
			RuntimeDependencies.Add("$(TargetOutputDir)/" + soName, pqSrc);

			// Optional deps: prefer lib/, fall back to bin/
			string[] deps = [ "libssl.so.3", "libcrypto.so.3 "];
			foreach (var dep in deps)
			{
				var depFromLib = Path.Combine(libPath, dep);
				var depFromBin = Path.Combine(binPath, dep);
				var depSrc = File.Exists(depFromLib) ? depFromLib : depFromBin;
				if (File.Exists(depSrc))
				{
					RuntimeDependencies.Add("$(TargetOutputDir)/" + dep, depSrc);
				}
			}
		}
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            var libPath = Path.Combine(thirdPartyPath, "lib", "Mac");
            var binPath = Path.Combine(thirdPartyPath, "bin", "Mac");

            PublicAdditionalLibraries.Add(Path.Combine(libPath, "libpq.dylib"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libpq.dylib", Path.Combine(binPath, "libpq.dylib"));

            string[] deps = [ "libssl.3.dylib", "libcrypto.3.dylib" ];
            foreach (var dep in deps)
			{
				var depSrcLib = Path.Combine(libPath, dep);
				var depSrcBin = Path.Combine(binPath, dep);
				var depSrc = File.Exists(depSrcLib) ? depSrcLib : depSrcBin;
				if (File.Exists(depSrc))
				{
					RuntimeDependencies.Add("$(TargetOutputDir)/" + dep, depSrc);
				}
			}
        }
    }
}
