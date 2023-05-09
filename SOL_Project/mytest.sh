#!/bin/bash

if [ ! -e generafile ];
then
    echo "Compilare generafile, eseguibile mancante!";
    exit 1
fi
if [ ! -e farm ];
then
    echo "Compilare farm, eseguibile mancante!"
    exit 1
fi

#
# il file expected.txt contiene i risultati attesi per i file
# generati nel seguito con il programma generafile
#
cat > expected.txt <<EOF
64834211 file100.dat
103453975 file2.dat
153259244 file1.dat
193031985 testdir/testdir2/file150.dat
258119464 file116.dat
293718900 file3.dat
380867448 file5.dat
518290132 file17.dat
532900090 file117.dat
584164283 file4.dat
748176663 file16.dat
890024017 testdir/file19.dat
985077644 testdir/file8.dat
1146505381 file10.dat
1485251680 file12.dat
1674267050 file13.dat
1878386755 file14.dat
1884778221 testdir/testdir2/file111.dat
2086317503 file15.dat
2322416554 file18.dat
2560452408 file20.dat
EOF

#
# generafile genera i file file100.dat file150.dat file19.dat file116.dat...
# in modo deterministico
#
j=1
for i in 100 150 19 116 2 1 117 3 5 17 4 16 19 8 10 111 12 13 14 15 18 20; do
    ./generafile file$i.dat $(($i*11 + $j*117)) > /dev/null
    j=$(($j+3))
done

mkdir -p testdir
mv file19.dat file8.dat testdir
mkdir -p testdir/testdir2
mv file111.dat file150.dat testdir/testdir2

# esecuzione con 2 thread e coda lunga 1
./farm -n 2 -q 1 file* -d testdir | grep "file*" | awk '{print $1,$2}' | diff - expected.txt
echo " "
echo "__________TEST 1_____________"
echo " "
if [[ $? != 0 ]]; then
    echo "          FAILED"
else
    echo "          PASSED"
fi

# esecuzione con 8 thread e coda lunga 16
./farm -d testdir -n 8 -q 16 file* | grep "file*" | awk '{print $1,$2}' | diff - expected.txt
echo " "
echo "__________TEST 2_____________"
echo " "
if [[ $? != 0 ]]; then
    echo "          FAILED"
else
    echo "          PASSED"
fi

#
# esecuzione "rallentata" con 1 thread, dopo circa 5 secondi viene
# inviato il segnale SIGTERM (comando pkill) e si valuta l'exit status
# 
./farm -n 1 -d testdir -q 1 -t 1000 file* 2>&1 > /dev/null &
pid=$!
echo " "
echo "__________TEST 3_____________"
echo " "
sleep 5
pkill farm
wait $pid
if [[ $? != 0 ]]; then
    echo "          FAILED"
else
    echo "          PASSED"
fi

#
# esecuzione con valgrind. Se valgrind trova dei problemi esce con 
# exit status 1.
#
valgrind --error-exitcode=1 --log-file=/dev/null ./farm -d testdir file* 2>&1 > /dev/null
echo " "
echo "__________TEST 4_____________"
echo " "
if [[ $? != 0 ]]; then
    echo "          FAILED"
else
    echo "          PASSED"
fi

# verifica di memory leaks
valgrind --leak-check=full --error-exitcode=1 --log-file=/dev/null ./farm file* -d testdir 2>&1 > /dev/null
echo " "
echo "__________TEST 5_____________"
echo " "
if [[ $? != 0 ]]; then
    echo "          FAILED"
else
    echo "          PASSED"
fi

# test che lancia 4 volte SIGUSR1
./farm -n 1 -d testdir -q 1 -t 1000 file* 2>&1 &
pid=$!
echo " "
echo "__________TEST 6_____________"
echo " "
for i in 1 2 3 4 5;
do
    sleep 3
    echo " "
    echo "($i/5) kill -USR1 $pid"
    echo " "
    kill -USR1 $pid
done
    sleep 1
    echo " "
    echo "COLLECTOR:"
    echo " "

wait $pid

#lancia 2 volte il segnale SIGUSR1 e dopo lancia SIGINT per farlo terminare
valgrind --leak-check=full --log-file=/dev/null ./farm -n 1 -d testdir -q 1 -t 1000 file* 2>&1 &
pid=$!
echo " "
echo "__________TEST 7_____________"
echo " "
for i in 1 2;
do
    sleep 5
    echo " "
    echo "($i/3) kill -USR1 $pid"
    echo " "
    kill -USR1 $pid
done
    sleep 2
    echo " "
    echo "(3/3) kill -INT $pid"
    echo " "
    echo "COLLECTOR:"
    echo " "
    kill -INT $pid

wait $pid

#verifica che alla ricezione di due segnali quasi nello stesso momento, vengano gestiti entrambi
valgrind --leak-check=full --log-file=/dev/null ./farm -n 1 -d testdir -q 4 -t 1000 file* 2>&1 &
pid=$!
echo " "
echo "__________TEST 8_____________"
echo " "
sleep 5
echo " "
echo "(1/2) kill -USR1 $pid"
echo " "
kill -USR1 $pid
echo "(2/2) kill -INT $pid"
echo " "
kill -INT $pid
echo " "
echo "COLLECTOR:"
echo " "
wait $pid


# come il test 3 ma eseguito con valgrind per verificare memory leaks
valgrind --leak-check=full --error-exitcode=1 --log-file=/dev/null ./farm -n 1 -d testdir -q 4 -t 1000 file* 2>&1 > /dev/null &
pid=$!
echo " "
echo "__________TEST 9_____________"
echo " "
sleep 4
pkill farm
wait $pid
if [[ $? != 0 ]]; then
    echo "          FAILED"
else
    echo "          PASSED"
fi

#test passando solo i file generati
valgrind --leak-check=full --error-exitcode=1 --log-file=/dev/null ./farm file* 2>&1 > /dev/null &
pid=$!
echo " "
echo "__________TEST 10_____________"
echo " "
wait $pid
if [[ $? != 0 ]]; then
    echo "          FAILED"
else
    echo "          PASSED"
fi

# test passando solo l' argomento -d testdir
valgrind --leak-check=full --error-exitcode=1 --log-file=/dev/null ./farm -d testdir 2>&1 > /dev/null &
pid=$!
echo " "
echo "__________TEST 11_____________"
echo " "
wait $pid
if [[ $? != 0 ]]; then
    echo "          FAILED"
else
    echo "          PASSED"
fi