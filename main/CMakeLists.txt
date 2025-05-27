
set(COMPONENT_SRCS
    "main.cc"
    "image_provider.cc"
    "tf_model.cpp"
    "pipeline_runner.cpp"
    "tf_model_data.cc"
)

set(COMPONENT_ADD_INCLUDEDIRS ". include")


register_component()
