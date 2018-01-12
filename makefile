build : main.o market.o datasource.o public.o log.o
	clang++ -l boost_system -l boost_filesystem -l boost_date_time -std=c++11 -o build main.o market.o datasource.o public.o log.o
main.o : main.cpp market.h
	clang++ -std=c++11 -g -c main.cpp
market.o : market.h market.cpp datasource.h public.h
	clang++ -std=c++11 -g -c market.cpp
datasource.o : datasource.h datasource.cpp public.h
	clang++ -std=c++11 -g -c datasource.cpp
public.o : public.h public.cpp log.h
	clang++ -std=c++11 -g -c public.cpp
log.o : log.h log.cpp
	clang++ -std=c++11 -g -c log.cpp
clean :
	rm main.o market.o datasource.o public.o log.o build