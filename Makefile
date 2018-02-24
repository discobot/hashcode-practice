B=cmake-build-relwithdebinfo

default:
	./$B/run streaming/me_at_the_zoo.in streaming/me_at_the_zoo.out
	python3 score.py streaming/me_at_the_zoo.in streaming/me_at_the_zoo.out
	./$B/run streaming/videos_worth_spreading.in streaming/videos_worth_spreading.out
	python3 score.py streaming/videos_worth_spreading.in streaming/videos_worth_spreading.out
	./$B/run streaming/trending_today.in streaming/trending_today.out
	python3 score.py streaming/trending_today.in streaming/trending_today.out
	./$B/run streaming/kittens.in.txt streaming/kittens.out
	python3 score.py streaming/kittens.in.txt streaming/kittens.out

