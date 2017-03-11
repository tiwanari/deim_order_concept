#!/bin/sh

dst=g1

# all
# ./create_dataset_with_global_vecs.rb ver24/original/all/all/k0   ver24/$dst/all/all/k0   list_file
# ./create_dataset_with_global_vecs.rb ver24/original/all/all/k0_1 ver24/$dst/all/all/k0_1 list_file
# ./create_dataset_with_global_vecs.rb ver24/original/all/all/k0_2 ver24/$dst/all/all/k0_2 list_file
# ./create_dataset_with_global_vecs.rb ver24/original/all/all/k0_3 ver24/$dst/all/all/k0_3 list_file

echo "ver24/original/all/all/k0" > list_file
./create_dataset_with_global_vecs.rb ver24/original/all/men/k0   ver24/$dst/all/men/k0   list_file
echo "ver24/original/all/all/k0_1" > list_file
./create_dataset_with_global_vecs.rb ver24/original/all/men/k0_1 ver24/$dst/all/men/k0_1 list_file
echo "ver24/original/all/all/k0_2" > list_file
./create_dataset_with_global_vecs.rb ver24/original/all/men/k0_2 ver24/$dst/all/men/k0_2 list_file
echo "ver24/original/all/all/k0_3" > list_file
./create_dataset_with_global_vecs.rb ver24/original/all/men/k0_3 ver24/$dst/all/men/k0_3 list_file

echo "ver24/original/all/all/k0" > list_file
./create_dataset_with_global_vecs.rb ver24/original/all/women/k0   ver24/$dst/all/women/k0   list_file
echo "ver24/original/all/all/k0_1" > list_file
./create_dataset_with_global_vecs.rb ver24/original/all/women/k0_1 ver24/$dst/all/women/k0_1 list_file
echo "ver24/original/all/all/k0_2" > list_file
./create_dataset_with_global_vecs.rb ver24/original/all/women/k0_2 ver24/$dst/all/women/k0_2 list_file
echo "ver24/original/all/all/k0_3" > list_file
./create_dataset_with_global_vecs.rb ver24/original/all/women/k0_3 ver24/$dst/all/women/k0_3 list_file


# kanto
echo "ver24/original/all/all/k0" > list_file
./create_dataset_with_global_vecs.rb ver24/original/kanto/all/k0   ver24/$dst/kanto/all/k0   list_file
echo "ver24/original/all/all/k0_1" > list_file
./create_dataset_with_global_vecs.rb ver24/original/kanto/all/k0_1 ver24/$dst/kanto/all/k0_1 list_file
echo "ver24/original/all/all/k0_2" > list_file
./create_dataset_with_global_vecs.rb ver24/original/kanto/all/k0_2 ver24/$dst/kanto/all/k0_2 list_file
echo "ver24/original/all/all/k0_3" > list_file
./create_dataset_with_global_vecs.rb ver24/original/kanto/all/k0_3 ver24/$dst/kanto/all/k0_3 list_file

echo "ver24/original/all/all/k0" > list_file
echo "ver24/original/kanto/all/k0" >> list_file
echo "ver24/original/all/men/k0" >> list_file
./create_dataset_with_global_vecs.rb ver24/original/kanto/men/k0   ver24/$dst/kanto/men/k0   list_file
echo "ver24/original/all/all/k0_1" > list_file
echo "ver24/original/kanto/all/k0_1" >> list_file
echo "ver24/original/all/men/k0_1" >> list_file
./create_dataset_with_global_vecs.rb ver24/original/kanto/men/k0_1 ver24/$dst/kanto/men/k0_1 list_file
echo "ver24/original/all/all/k0_2" > list_file
echo "ver24/original/kanto/all/k0_2" >> list_file
echo "ver24/original/all/men/k0_2" >> list_file
./create_dataset_with_global_vecs.rb ver24/original/kanto/men/k0_2 ver24/$dst/kanto/men/k0_2 list_file
echo "ver24/original/all/all/k0_3" > list_file
echo "ver24/original/kanto/all/k0_3" >> list_file
echo "ver24/original/all/men/k0_3" >> list_file
./create_dataset_with_global_vecs.rb ver24/original/kanto/men/k0_3 ver24/$dst/kanto/men/k0_3 list_file

echo "ver24/original/all/all/k0" > list_file
echo "ver24/original/kanto/all/k0" >> list_file
echo "ver24/original/all/women/k0" >> list_file
./create_dataset_with_global_vecs.rb ver24/original/kanto/women/k0   ver24/$dst/kanto/women/k0   list_file
echo "ver24/original/all/all/k0_1" > list_file
echo "ver24/original/kanto/all/k0_1" >> list_file
echo "ver24/original/all/women/k0_1" >> list_file
./create_dataset_with_global_vecs.rb ver24/original/kanto/women/k0_1 ver24/$dst/kanto/women/k0_1 list_file
echo "ver24/original/all/all/k0_2" > list_file
echo "ver24/original/kanto/all/k0_2" >> list_file
echo "ver24/original/all/women/k0_2" >> list_file
./create_dataset_with_global_vecs.rb ver24/original/kanto/women/k0_2 ver24/$dst/kanto/women/k0_2 list_file
echo "ver24/original/all/all/k0_3" > list_file
echo "ver24/original/kanto/all/k0_3" >> list_file
echo "ver24/original/all/women/k0_3" >> list_file
./create_dataset_with_global_vecs.rb ver24/original/kanto/women/k0_3 ver24/$dst/kanto/women/k0_3 list_file


# kinki
echo "ver24/original/all/all/k0" > list_file
./create_dataset_with_global_vecs.rb ver24/original/kinki/all/k0   ver24/$dst/kinki/all/k0   list_file
echo "ver24/original/all/all/k0_1" > list_file
./create_dataset_with_global_vecs.rb ver24/original/kinki/all/k0_1 ver24/$dst/kinki/all/k0_1 list_file
echo "ver24/original/all/all/k0_2" > list_file
./create_dataset_with_global_vecs.rb ver24/original/kinki/all/k0_2 ver24/$dst/kinki/all/k0_2 list_file
echo "ver24/original/all/all/k0_3" > list_file
./create_dataset_with_global_vecs.rb ver24/original/kinki/all/k0_3 ver24/$dst/kinki/all/k0_3 list_file

echo "ver24/original/all/all/k0" > list_file
echo "ver24/original/kinki/all/k0" >> list_file
echo "ver24/original/all/men/k0" >> list_file
./create_dataset_with_global_vecs.rb ver24/original/kinki/men/k0   ver24/$dst/kinki/men/k0   list_file
echo "ver24/original/all/all/k0_1" > list_file
echo "ver24/original/kinki/all/k0_1" >> list_file
echo "ver24/original/all/men/k0_1" >> list_file
./create_dataset_with_global_vecs.rb ver24/original/kinki/men/k0_1 ver24/$dst/kinki/men/k0_1 list_file
echo "ver24/original/all/all/k0_2" > list_file
echo "ver24/original/kinki/all/k0_2" >> list_file
echo "ver24/original/all/men/k0_2" >> list_file
./create_dataset_with_global_vecs.rb ver24/original/kinki/men/k0_2 ver24/$dst/kinki/men/k0_2 list_file
echo "ver24/original/all/all/k0_3" > list_file
echo "ver24/original/kinki/all/k0_3" >> list_file
echo "ver24/original/all/men/k0_3" >> list_file
./create_dataset_with_global_vecs.rb ver24/original/kinki/men/k0_3 ver24/$dst/kinki/men/k0_3 list_file

echo "ver24/original/all/all/k0" > list_file
echo "ver24/original/kinki/all/k0" >> list_file
echo "ver24/original/all/women/k0" >> list_file
./create_dataset_with_global_vecs.rb ver24/original/kinki/women/k0   ver24/$dst/kinki/women/k0   list_file
echo "ver24/original/all/all/k0_1" > list_file
echo "ver24/original/kinki/all/k0_1" >> list_file
echo "ver24/original/all/women/k0_1" >> list_file
./create_dataset_with_global_vecs.rb ver24/original/kinki/women/k0_1 ver24/$dst/kinki/women/k0_1 list_file
echo "ver24/original/all/all/k0_2" > list_file
echo "ver24/original/kinki/all/k0_2" >> list_file
echo "ver24/original/all/women/k0_2" >> list_file
./create_dataset_with_global_vecs.rb ver24/original/kinki/women/k0_2 ver24/$dst/kinki/women/k0_2 list_file
echo "ver24/original/all/all/k0_3" > list_file
echo "ver24/original/kinki/all/k0_3" >> list_file
echo "ver24/original/all/women/k0_3" >> list_file
./create_dataset_with_global_vecs.rb ver24/original/kinki/women/k0_3 ver24/$dst/kinki/women/k0_3 list_file
