{
    "targets": [
        {
            "target_name": "somhunter_core",
            "sources": [
                "SOMHunterCore/src/json11.cpp",
                "main.cpp",
                "SomHunterWrapper.cpp",
                "SOMHunterCore/src/SOM.cpp",
                "SOMHunterCore/src/AsyncSom.cpp",
                "SOMHunterCore/src/Features.cpp",
                "SOMHunterCore/src/Frames.cpp",
                "SOMHunterCore/src/ImageKeywordsW2VV.cpp",
                "SOMHunterCore/src/Scores.cpp",
                "SOMHunterCore/src/SomHunter.cpp",
                "SOMHunterCore/src/Submitter.cpp",
            ],
            "include_dirs": [
                "<!@(node -p \"require('node-addon-api').include\")",
                "SOMHunterCore/src/"
            ],
            "libraries": [],
            "dependencies": [
                "<!(node -p \"require('node-addon-api').gyp\")"
            ],
            "defines": [
                "NAPI_CPP_EXCEPTIONS",
                "HAS_NAPI_HEADERS"
            ],
            "cflags_cc": [
                "-std=c++17","-fexceptions","-Wall", "-march=native"
            ],
            "msvs_settings": {
                "VCCLCompilerTool": {
                    "AdditionalOptions": [
                        "-std:c++17"
                    ]
                }
            },
            "conditions": [
                [
                    "OS=='linux'",
                    {
                        "include_dirs":[
                            "<!@(pkg-config libcurl --cflags-only-I | sed s/-I//g)"
                        ],
                        "link_settings": {
                            "libraries": [
                                "-L/usr/lib64/",
                                "<!@(pkg-config libcurl --libs)"
                            ]
                        }
                    }
                ],
                [
                    "OS=='win'",
                    {
                        "include_dirs":[
                            "C:\\Program Files\\curl\\include\\"
                        ],
                        "link_settings": {
                            "libraries": [
                                "libcurl.lib",
                                "zlib.lib"
                            ]
                        },
                        "msvs_settings": {
                            "VCCLCompilerTool": {
                                "AdditionalOptions": [
                                    "-std:c++17",
                                    "/MP /EHsc /Qspectre"
                                ]
                            },
                            "VCLinkerTool": {
                                "AdditionalLibraryDirectories": [
                                    "C:\\Program Files\\curl\\lib\\"
                                ]
                            }
                        }
                    }
                ]
            ]
        }
    ]
}