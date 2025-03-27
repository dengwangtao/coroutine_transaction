# protoc error_define.proto proto_base.proto --cpp_out=.


proto_path="/home/dengwangtao/codes/coroutine_transaction/src/proto"

if [ $# -eq 0 ]; then
    echo "Usage: $0 clear/build"
    exit 1
fi

case $1 in
    clear)
        rm -rf ${proto_path}/*.pb.h ${proto_path}/*.pb.cc
        ;;
    build)
        ls ${proto_path} | grep -E ".*\.proto$" | xargs protoc --cpp_out=${proto_path} --proto_path=${proto_path}
        ;;
    *)
        echo "Usage: $0 clear/build"
        exit 1
        ;;
esac

