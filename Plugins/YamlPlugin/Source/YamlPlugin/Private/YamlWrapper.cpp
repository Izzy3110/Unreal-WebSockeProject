#include "YamlWrapper.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

// yaml-cpp includes
#include "yaml-cpp/yaml.h"

bool FYamlWrapper::ParseYamlToMap(const FString& YamlText, TMap<FString, FString>& OutMap)
{
    OutMap.Empty();

    // Convert FString -> std::string
    std::string stdStr(TCHAR_TO_UTF8(*YamlText));

    try
    {
        YAML::Node root = YAML::Load(stdStr);
        if (!root.IsMap())
        {
            return false;
        }

        for (auto it = root.begin(); it != root.end(); ++it)
        {
            YAML::Node keyNode = it->first;
            YAML::Node valNode = it->second;

            if (!keyNode.IsScalar()) continue;

            std::string key = keyNode.as<std::string>();

            FString KeyF = FString(UTF8_TO_TCHAR(key.c_str()));
            FString ValF;

            // handle scalar or sequence or map -> convert to string
            if (valNode.IsScalar())
            {
                std::string val = valNode.as<std::string>();
                ValF = FString(UTF8_TO_TCHAR(val.c_str()));
            }
            else
            {
                // fallback: emit node to string
                YAML::Emitter out;
                out << valNode;
                std::string emitted = out.c_str();
                ValF = FString(UTF8_TO_TCHAR(emitted.c_str()));
            }

            OutMap.Add(KeyF, ValF);
        }
    }
    catch (const YAML::Exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("YAML parse error: %s"), UTF8_TO_TCHAR(e.what()));
        return false;
    }

    return true;
}

bool FYamlWrapper::EmitMapToYaml(const TMap<FString, FString>& Map, FString& OutYaml)
{
    YAML::Emitter out;
    out << YAML::BeginMap;
    for (const auto& Pair : Map)
    {
        std::string key(TCHAR_TO_UTF8(*Pair.Key));
        std::string val(TCHAR_TO_UTF8(*Pair.Value));
        out << YAML::Key << key << YAML::Value << val;
    }
    out << YAML::EndMap;

    OutYaml = FString(UTF8_TO_TCHAR(out.c_str()));
    return true;
}
