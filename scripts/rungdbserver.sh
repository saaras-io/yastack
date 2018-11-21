ps -ef | grep ev-source-exe && sudo gdbserver localhost:2345 --attach `pidof -s ev-source-exe`
