#include "dirut.h"
#include "iterator.h"

#include <library/unittest/registar.h>

#include <util/system/fs.h>
#include <util/system/file.h>
#include <util/generic/hash.h>

class TDirIteratorTest: public TTestBase {
        UNIT_TEST_SUITE(TDirIteratorTest);
            UNIT_TEST(TestIt)
            UNIT_TEST(TestError)
            UNIT_TEST(TestLocal)
        UNIT_TEST_SUITE_END();
    private:
        class TDirHier {
            public:
                struct TPath {
                    Stroka Path;
                    int Type;
                };

                inline void Add(const TPath& path) {
                    switch (path.Type) {
                        case 0:
                            TFile(path.Path, CreateAlways | RdWr);
                            break;

                        case 1:
                            MakeDirIfNotExist(~path.Path);
                            break;

                        case 2:
                            ythrow yexception() << "shit happen";
                    }

                    Paths_.push_back(path);
                    Srch_[path.Path] = path;
                }

                inline int Type(const Stroka& path) {
                    yhash<Stroka, TPath>::const_iterator it = Srch_.find(path);

                    UNIT_ASSERT(it != Srch_.end());

                    return it->second.Type;
                }

                inline bool Have(const Stroka& path, int type) {
                    return Type(path) == type;
                }

                inline ~TDirHier() {
                    for (size_t i = 0; i < Paths_.size(); ++i) {
                        NFs::Remove(~Paths_[Paths_.size() - i - 1].Path);
                    }
                }

            private:
                yvector<TPath> Paths_;
                yhash<Stroka, TPath> Srch_;
        };

        inline void TestLocal() {
            Stroka dirname("."LOCSLASH_S);
            TDirIterator d(dirname, FTS_NOCHDIR);
            for (TDirIterator::TIterator it = d.Begin(); it != d.End(); ++it) {
            }
        }

        inline void TestIt() {
            TDirHier hier;

            const Stroka dir = "tmpdir";
            const TDirHier::TPath path = {dir, 1};

            hier.Add(path);

            for (size_t i = 0; i < 10; ++i) {
                const Stroka dir1 = dir + LOCSLASH_C + ToString(i);
                const TDirHier::TPath path1 = {dir1, 1};

                hier.Add(path1);

                for (size_t j = 0; j < 10; ++j) {
                    const Stroka subdir2 = ToString(j);
                    const Stroka dir2 = dir1 + LOCSLASH_C + subdir2;
                    const TDirHier::TPath path2 = {dir2, 1};

                    hier.Add(path2);

                    for (size_t k = 0; k < 3; ++k) {
                        const Stroka file = dir2 + LOCSLASH_C + "file" + ToString(k);
                        const TDirHier::TPath fpath = {file, 0};

                        hier.Add(fpath);
                    }
                }
            }

            TDirIterator d(dir);

            for (TDirIterator::TIterator it = d.Begin(); it != d.End(); ++it) {
                UNIT_ASSERT(hier.Have(it->fts_path, it->fts_info != FTS_F));
            }
        }

        inline void TestError() {
            try {
                TDirIterator d("./notexistingfilename");

                UNIT_ASSERT(false);
            } catch (const TDirIterator::TError&) {
            } catch (...) {
                UNIT_ASSERT(false);
            }

            UNIT_ASSERT(true);
        }
};

UNIT_TEST_SUITE_REGISTRATION(TDirIteratorTest);