

if [ $# -eq 0 ]; then
    echo "Usage: $0 clear/build"
    exit 1
fi

case $1 in
    clear)
        rm -rf *.pb.h *.pb.cc
        ;;
    build)
        rm -rf *.pb.h *.pb.cc
        ls | grep -E ".*\.proto$" | xargs protoc --cpp_out=. --proto_path=.
        ;;
    *)
        echo "Usage: $0 clear/build"
        exit 1
        ;;
esac

