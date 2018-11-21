if [ $# -gt 1 ]; then

	cn='localhost'
	cn=$1

	echo "Received cn as" $1
	openssl req -new -newkey rsa:$2 -days 36500 -nodes -x509 \
	    -subj "/C=US/ST=Denial/L=Springfield/O=Dis/CN=$cn" \
	    -keyout $1.key  -out $1.cert
	
	openssl x509 -in $cn.cert -text -noout
	
#	cn2='127.0.0.1'
#	openssl req -new -newkey rsa:4096 -days 36500 -nodes -x509 \
#	    -subj "/C=US/ST=Denial/L=Springfield/O=Dis/CN=$cn2" \
#	    -keyout 127001.key  -out 127001.cert
#	
#	openssl x509 -in 127001.cert -text -noout
#	
#	cn3='172.31.9.84'
#	openssl req -new -newkey rsa:4096 -days 36500 -nodes -x509 \
#	    -subj "/C=US/ST=Denial/L=Springfield/O=Dis/CN=$cn3" \
#	    -keyout $cn3.key  -out $cn3.cert
#	
#	openssl x509 -in $cn3.cert -text -noout

else
	echo "Script to generate self signed certificate with a provided CN name and keysize"
	echo "On successful execution, output is <cn_name.key> and <cn_name.cert>"
	echo "Usage:    ./self_signed_cert.sh <cn_name> <keysize>"
	echo "Example:  ./self_signed_cert.sh vs.app 4096"
	echo "Example:  ./self_signed_cert.sh vs.app 2048"
fi
