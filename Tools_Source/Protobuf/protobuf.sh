#! /bin/sh
echo "Start Generate cpp file,directory is out"
for file in ./out/*
do
	rm -rf $file
done

cd proto
for file in ./*.proto
do
	if test -f $file; then		
		protoc --cpp_out=. $file
		echo "$file"
	fi
done
for file in ./*.h
do
	mv $file ../out/
done
for file in ./*.cc
do
	mv $file ../out/
done
cd ../
rm -Rf /home/xuke/Project/Git_Client/Server/C++_Server_EclipseProject/src/Proto.cpp/* 

rm -Rf /home/xuke/Project/Git_Client/Server/C++_Server_EclipseProject/src/Proto/* 

cp -Rf out/* /home/xuke/Project/Git_Client/Server/C++_Server_EclipseProject/src/Proto.cpp/
cp -Rf proto/* /home/xuke/Project/Git_Client/Server/C++_Server_EclipseProject/src/Proto/

echo "Finsh" 
