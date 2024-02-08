idf_component_register(SRCS "main.c"
                            "rest_server.c"
                            "db.c"
                            "rgb.c"
                    INCLUDE_DIRS "."

                    EMBED_TXTFILES ../website/index.html ../website/style.css ../website/script.js)