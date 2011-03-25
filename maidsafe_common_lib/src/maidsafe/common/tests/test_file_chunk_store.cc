/* Copyright (c) 2011 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <memory>
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"
#include "maidsafe/common/tests/test_chunk_store_api.h"
#include "maidsafe/common/file_chunk_store.h"

namespace maidsafe {

namespace test {

template<>
void ChunkStoreTest<FileChunkStore>::InitChunkStore(
    std::shared_ptr<ChunkStore> chunk_store, const fs::path &chunk_dir) {
  reinterpret_cast<FileChunkStore*>(chunk_store.get())->Init(chunk_dir);
}

INSTANTIATE_TYPED_TEST_CASE_P(Files, ChunkStoreTest, FileChunkStore);

class FileChunkStoreTest: public testing::Test {
 public:
  FileChunkStoreTest()
      : test_dir_(fs::unique_path(fs::temp_directory_path() /
                  "MaidSafe_TestFileChunkStore_%%%%-%%%%-%%%%")),
        chunk_dir_(test_dir_ / "chunks"),
        ref_chunk_dir_(test_dir_ / "ref_chunks") {}
  ~FileChunkStoreTest() {}
 protected:
  void SetUp() {
    if (fs::exists(test_dir_))
      fs::remove_all(test_dir_);
    fs::create_directories(chunk_dir_);
    fs::create_directories(ref_chunk_dir_);
  }
  void TearDown() {
    try {
      if (fs::exists(test_dir_))
        fs::remove_all(test_dir_);
    }
    catch(...) {}
  }
  fs::path test_dir_, chunk_dir_, ref_chunk_dir_;
};

TEST_F(FileChunkStoreTest, BEH_FCS_Init) {
  //  File chunk store without reference counting
  std::shared_ptr<FileChunkStore> fcs_first(new FileChunkStore(false));

  fs::path chunk_dir_first(test_dir_ / "chunks_first");
  EXPECT_EQ(true, fcs_first->Init(chunk_dir_first, 10));
  EXPECT_EQ(0, fcs_first->Count());
  EXPECT_TRUE(fcs_first->Empty());
  EXPECT_FALSE(fcs_first->Has(""));
  EXPECT_FALSE(fcs_first->Has("something"));

  //  Reuse existing chunk directory
  std::shared_ptr<FileChunkStore> fcs_second(new FileChunkStore(false));
  EXPECT_EQ(true, fcs_second->Init(chunk_dir_first, 10));
  EXPECT_EQ(0, fcs_second->Count());
  EXPECT_TRUE(fcs_second->Empty());
  EXPECT_FALSE(fcs_second->Has(""));
  EXPECT_FALSE(fcs_second->Has("something"));

  //  Test by passing nothing for Dir name
  std::shared_ptr<FileChunkStore> fcs_third(new FileChunkStore(false));
  EXPECT_EQ(false, fcs_third->Init("", 10));
  EXPECT_EQ(0, fcs_third->Count());
  EXPECT_TRUE(fcs_third->Empty());
  EXPECT_FALSE(fcs_third->Has(""));
  EXPECT_FALSE(fcs_third->Has("something"));

  //  Test initialiation of reference counted file chunk store
  std::shared_ptr<FileChunkStore> ref_fcs_first(new FileChunkStore(true));

  fs::path ref_chunk_dir_first(test_dir_ / "ref_chunks_first");
  EXPECT_EQ(true, ref_fcs_first->Init(ref_chunk_dir_first, 10));
  EXPECT_EQ(0, ref_fcs_first->Count());
  EXPECT_TRUE(ref_fcs_first->Empty());
  EXPECT_FALSE(ref_fcs_first->Has(""));
  EXPECT_FALSE(ref_fcs_first->Has("something"));

  //  Reuse existing chunk directory
  std::shared_ptr<FileChunkStore> ref_fcs_second(new FileChunkStore(true));
  EXPECT_EQ(true, ref_fcs_second->Init(ref_chunk_dir_first, 10));
  EXPECT_EQ(0, ref_fcs_second->Count());
  EXPECT_TRUE(ref_fcs_second->Empty());
  EXPECT_FALSE(ref_fcs_second->Has(""));
  EXPECT_FALSE(ref_fcs_second->Has("something"));

  //  Test by passing nothing for Dir name
  std::shared_ptr<FileChunkStore> ref_fcs_third(new FileChunkStore(true));
  EXPECT_EQ(false, ref_fcs_third->Init("", 10));
  EXPECT_EQ(0, ref_fcs_third->Count());
  EXPECT_TRUE(ref_fcs_third->Empty());
  EXPECT_FALSE(ref_fcs_third->Has(""));
  EXPECT_FALSE(ref_fcs_third->Has("something"));
}

TEST_F(FileChunkStoreTest, BEH_FCS_Get) {
  std::shared_ptr<FileChunkStore> fcs(new FileChunkStore(false));

  std::string content(RandomString(100));
  std::string name(crypto::Hash<crypto::SHA512>(content));
  fs::path path(test_dir_ / "chunk.dat");

  //  try to get a chunk without initialising chunk store
  EXPECT_TRUE(fcs->Get("anything").empty());
  EXPECT_EQ(false, fcs->Get("some_chunk", path));

  //  initialise
  fcs->Init(chunk_dir_, 2);

  //  try getting something non existing
  EXPECT_TRUE(fcs->Get("whatever").empty());

  //  store data
  ASSERT_TRUE(fcs->Store(name, content));

  // existing chunk
  EXPECT_EQ(content, fcs->Get(name));
  EXPECT_TRUE(fcs->Get(name, path));

  //  create a ref counted chunk store
  std::shared_ptr<FileChunkStore> fcs_ref(new FileChunkStore(true));
  EXPECT_EQ(true, fcs_ref->Init(ref_chunk_dir_, 10));
  ASSERT_TRUE(fcs_ref->Store(name, content));
  ASSERT_TRUE(fcs_ref->Store(name, content));
  ASSERT_TRUE(fcs_ref->Store(name, content));

  //  get the chunk
  fs::path sink_path(test_dir_ / "my_chunk.dat");
  EXPECT_FALSE(fs::exists(sink_path));
  EXPECT_TRUE(fcs_ref->Get(name, sink_path));
}

TEST_F(FileChunkStoreTest, BEH_FCS_Store) {
  std::shared_ptr<FileChunkStore> fcs(new FileChunkStore(false));

  std::string content(RandomString(100));
  std::string name(crypto::Hash<crypto::SHA512>(content));

  //  try storing without initialising
  EXPECT_FALSE(fcs->Store(name, content));
  EXPECT_FALSE(fcs->Store(name, fs::path("anypath"), true));
  EXPECT_TRUE(fcs->Init(chunk_dir_, 4));

  //  try storing an empty chunk
  EXPECT_FALSE(fcs->Store("", content));
  EXPECT_FALSE(fcs->Store(name, ""));

  //  try storing a chunk
  EXPECT_TRUE(fcs->Store(name, content));
  //  same one again
  EXPECT_TRUE(fcs->Store(name, content));

  fs::path path(test_dir_ / "chunk.dat");
  EXPECT_TRUE(fcs->Get(name, path));

  //  reference counted chunk store
  boost::shared_ptr<FileChunkStore> ref_fcs(new FileChunkStore(true));
  EXPECT_FALSE(ref_fcs->Store(name, content));

  EXPECT_TRUE(ref_fcs->Init(ref_chunk_dir_, 3));
  fs::path non_existant_file(test_dir_ / "non_existant");
  EXPECT_FALSE(ref_fcs->Store(name, non_existant_file, true));

  //  store chunks iteratively
  int count = 1000;
  for (int iter = 0; iter < count; ++iter) {
    std::string content(RandomString(500));
    std::string name(crypto::Hash<crypto::SHA512>(content));
    std::string file_name(EncodeToHex(RandomString(10)));
    fs::path path(test_dir_ / file_name);

    EXPECT_TRUE(ref_fcs->Store(name, content));
    EXPECT_TRUE(ref_fcs->Get(name, path));
    EXPECT_TRUE(ref_fcs->Store(name, path, true));

    std::string content1(RandomString(500));
    std::string name1(crypto::Hash<crypto::SHA512>(content1));
    std::string file_name1(EncodeToHex(RandomString(10)));
    fs::path path1(test_dir_ / file_name1);

    EXPECT_TRUE(ref_fcs->Store(name1, content1));
    EXPECT_TRUE(ref_fcs->Get(name1, path1));
    EXPECT_TRUE(ref_fcs->Store(name1, path1, false));
  }

  //  reuse ref_fcs chunk store
  std::shared_ptr<FileChunkStore> reused_fcs(new FileChunkStore(true));
  EXPECT_TRUE(reused_fcs->Init(ref_chunk_dir_, 3));
}

TEST_F(FileChunkStoreTest, BEH_FCS_Capacity) {
  //  create a chunk store with limited capacity
  std::shared_ptr<FileChunkStore> fcs_cap(new FileChunkStore(true));
  EXPECT_TRUE(fcs_cap->Init(ref_chunk_dir_, 4));
  fcs_cap->SetCapacity(100);
  EXPECT_TRUE(fcs_cap->Empty());

  std::string content(RandomString(100));
  std::string name(crypto::Hash<crypto::SHA512>(content));
  std::string file_name("file.dat");
  fs::path path(test_dir_ / file_name);

  EXPECT_TRUE(fcs_cap->Store(name, content));

  std::string extra_content(RandomString(1));
  std::string extra_content_chunk_name(
                crypto::Hash<crypto::SHA512>(extra_content));
  EXPECT_FALSE(fcs_cap->Store(extra_content_chunk_name, extra_content));

  EXPECT_TRUE(fcs_cap->Get(name, path));
  EXPECT_FALSE(fcs_cap->Store(extra_content_chunk_name, path, true));
}

TEST_F(FileChunkStoreTest, BEH_FCS_Misc) {
  //  create a chunk store without reference counting
  std::shared_ptr<FileChunkStore> fcs(new FileChunkStore(false));

  EXPECT_TRUE(fcs->Init(chunk_dir_, 5));
  int count = 10;
  //  store chunks iteratively
  for (int i = 0; i < count; ++i) {
    std::string content(RandomString(500));
    std::string name(crypto::Hash<crypto::SHA512>(content));

    EXPECT_TRUE(fcs->Store(name, content));
  }

  std::string content("mycontent");
  std::string name(crypto::Hash<crypto::SHA512>(content));
  EXPECT_TRUE(fcs->Store(name, content));

  //  create a ref counted chunk store
  std::shared_ptr<FileChunkStore> ref_fcs(new FileChunkStore(true));
  EXPECT_TRUE(ref_fcs->Init(chunk_dir_, 5));

  fs::path path(test_dir_ / "chunk.dat");

  //  try to retrieve a chunk stored without reference count
  EXPECT_TRUE(ref_fcs->Get(name).empty());

}

TEST_F(FileChunkStoreTest, BEH_FCS_Delete) {
  std::shared_ptr<FileChunkStore> fcs(new FileChunkStore(false));
  EXPECT_FALSE(fcs->Delete("somechunk"));

  std::shared_ptr<FileChunkStore> ref_fcs(new FileChunkStore(true));
  EXPECT_TRUE(ref_fcs->Init(ref_chunk_dir_, 4));

  std::string content("mycontent");
  std::string name(crypto::Hash<crypto::SHA512>(content));
  EXPECT_TRUE(ref_fcs->Store(name, content));

  //  try deleting non existant chunk
  EXPECT_TRUE(ref_fcs->Delete("unknownchunk"));
}

TEST_F(FileChunkStoreTest, BEH_FCS_MoveTo) {
  std::shared_ptr<FileChunkStore> fcs(new FileChunkStore(false));
  std::shared_ptr<FileChunkStore> sink_fcs(new FileChunkStore(false));

  EXPECT_FALSE(fcs->MoveTo("somechunk", sink_fcs.get()));
}

TEST_F(FileChunkStoreTest, BEH_FCS_Validate) {
  std::shared_ptr<FileChunkStore> fcs(new FileChunkStore(false));
  EXPECT_FALSE(fcs->Validate("somechunk"));
}

TEST_F(FileChunkStoreTest, BEH_FCS_Size) {
  std::shared_ptr<FileChunkStore> fcs(new FileChunkStore(false));
  EXPECT_EQ(0, fcs->Size("somechunk"));

  EXPECT_TRUE(fcs->Init(chunk_dir_, 5));

  EXPECT_EQ(0, fcs->Size("somechunk"));
}

TEST_F(FileChunkStoreTest, BEH_FCS_Count) {
  std::shared_ptr<FileChunkStore> fcs(new FileChunkStore(false));
  EXPECT_EQ(0, fcs->Count("somechunk"));
}

}  // namespace test

}  // namespace maidsafe
