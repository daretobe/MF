import random as ram

ftrain = open("epinions.train","w")
ftest = open("epinions.test","w")

with open("epinions_rating.data") as f:
	for line in f:
		coin = ram.random()
		if coin>0.2:
			ftrain.write(line)
		else:
			ftest.write(line)
f.close()
ftrain.close()
ftest.close()