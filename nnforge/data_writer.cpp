/*
 *  Copyright 2011-2014 Maxim Milakov
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "data_writer.h"

#include "rnd.h"
#include "neural_network_exception.h"

namespace nnforge
{
	data_writer::data_writer()
	{
	}

	data_writer::~data_writer()
	{
	}

	void data_writer::write_randomized(unsupervised_data_reader& reader)
	{
		unsigned int entry_count = reader.get_entry_count();
		if (entry_count == 0)
			return;

		random_generator rnd = rnd::get_random_generator();

		std::vector<unsigned int> entry_to_write_list(entry_count);
		for(unsigned int i = 0; i < entry_count; ++i)
		{
			entry_to_write_list[i] = i;
		}

		std::vector<unsigned char> entry_data;

		for(unsigned int entry_to_write_count = entry_count; entry_to_write_count > 0; --entry_to_write_count)
		{
			nnforge_uniform_int_distribution<unsigned int> dist(0, entry_to_write_count - 1);

			unsigned int index = dist(rnd);
			unsigned int entry_id = entry_to_write_list[index];

			reader.rewind(entry_id);
			reader.raw_read(entry_data);
			raw_write(&(*entry_data.begin()), entry_data.size());

			unsigned int leftover_entry_id = entry_to_write_list[entry_to_write_count - 1];
			entry_to_write_list[index] = leftover_entry_id;
		}
	}

	void data_writer::write_randomized_classifier(supervised_data_reader& reader)
	{
		unsigned int entry_count = reader.get_entry_count();
		if (entry_count == 0)
			return;

		random_generator rnd = rnd::get_random_generator();

		std::vector<randomized_classifier_keeper> class_buckets_entry_id_lists;
		reader.fill_class_buckets_entry_id_lists(class_buckets_entry_id_lists);

		std::vector<unsigned char> entry_data;

		for(unsigned int entry_to_write_count = entry_count; entry_to_write_count > 0; --entry_to_write_count)
		{
			std::vector<randomized_classifier_keeper>::iterator bucket_it = class_buckets_entry_id_lists.begin();
			float best_ratio = 0.0F;
			for(std::vector<randomized_classifier_keeper>::iterator it = class_buckets_entry_id_lists.begin(); it != class_buckets_entry_id_lists.end(); ++it)
			{
				float new_ratio = it->get_ratio();
				if (new_ratio > best_ratio)
				{
					bucket_it = it;
					best_ratio = new_ratio;
				}
			}

			if (bucket_it->is_empty())
				throw neural_network_exception("Unexpected error in write_randomized_classifier: No elements left");

			unsigned int entry_id = bucket_it->peek_random(rnd);

			reader.rewind(entry_id);
			reader.raw_read(entry_data);
			raw_write(&(*entry_data.begin()), entry_data.size());
		}
	}
}
