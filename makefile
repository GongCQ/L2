build : main.o market.o datasource.o public.o log.o ddi.o ddi_min5.o big_small.o
	clang++ -l boost_system -l boost_filesystem -l boost_date_time -l gsl -l gslcblas -std=c++11 -O2 -o build main.o market.o datasource.o public.o log.o ddi.o ddi_min5.o big_small.o
main.o : main.cpp market.h ddi.h
	clang++ -std=c++11 -O2 -g -c main.cpp
market.o : market.h market.cpp datasource.h public.h
	clang++ -std=c++11 -O2 -g -c market.cpp
datasource.o : datasource.h datasource.cpp public.h
	clang++ -std=c++11 -O2 -g -c datasource.cpp
public.o : public.h public.cpp log.h 
	clang++ -std=c++11 -O2 -g -c public.cpp
log.o : log.h log.cpp
	clang++ -std=c++11 -O2 -g -c log.cpp
ddi.o : ddi.h ddi.cpp 
	clang++ -std=c++11 -O2 -g -c ddi.cpp
ddi_min5.o : ddi_min5.h ddi_min5.cpp 
	clang++ -std=c++11 -O2 -g -c ddi_min5.cpp
big_small.o : big_small.h big_small.cpp 
	clang++ -std=c++11 -O2 -g -c big_small.cpp
clean :
	rm main.o market.o datasource.o public.o log.o ddi.o ddi_min5.o big_small.o build