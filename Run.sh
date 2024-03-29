echo "Starting Server Instance"
pwd
node main.js > server_logs &

echo "Running Makefile"
pwd
make
echo "Running client instant."
./client.a > client_logs &

echo "Waiting 5 secs"
sleep 5
echo "Shutting down server and client."
pkill client
pkill node
