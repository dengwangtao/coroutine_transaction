# protoc error_define.proto proto_base.proto --cpp_out=.


ls | grep -E ".*\.proto$" | xargs protoc --cpp_out=.