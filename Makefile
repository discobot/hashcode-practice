B=cmake-build-relwithdebinfo

default:
	./$B/run streaming/kittens.in.txt streaming/kittens.out
	python score.py streaming/kittens.in.txt streaming/kittens.out
	./$B/run streaming/me_at_the_zoo.in streaming/me_at_the_zoo.out
	python score.py streaming/me_at_the_zoo.in streaming/me_at_the_zoo.out
	./$B/run streaming/trending_today.in streaming/trending_today.out
	python score.py streaming/trending_today.in streaming/trending_today.out
	./$B/run streaming/videos_worth_spreading.in streaming/videos_worth_spreading.out
	python score.py streaming/videos_worth_spreading.in streaming/videos_worth_spreading.out

