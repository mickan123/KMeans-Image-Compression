- Must compile with c++11 or better.
- Must set data_max and data_min constants at top of code
- Number of iterations can be set as desired although 
  for image compression it gets very slow quickly especially
  with a large number of clusters
- Test1, test2, and test3 are ppm image files used in the report

Call program with 2 command line arguments:
	argument 1 = number of clusters 
	argument 2 = name of data file (e.g. testdata1.txt)

e.g. ./exe 3 testdata1.txt
