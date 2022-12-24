#include "pch.h"
#include "fileSystem.h"
#include <cstdio>


namespace GSH {

    bool FileSystem::init(bool force_reformat) 
    {
        // Try to mount the filesystem
        GSH_INFO("Mounting the filesystem... ");
        int err = m_FS.mount(m_BD);
        GSH_INFO("%s\n", (err ? "Fail :(" : "OK"));
        if (err || force_reformat) {
            // Reformat if we can't mount the filesystem
            GSH_WARN("formatting... ");
            err = m_FS.reformat(m_BD);
            GSH_INFO("%s\n", (err ? "Fail :(" : "OK"));
            if (err) {
                GSH_ERROR("%s (%d)\n", strerror(-err), err);
                return false;
            }
        }

        m_CurrentDirPath = "/fs/";

        return true;
    }

    
    FILE* FileSystem::openFile(const std::string& filename, const char* flag)
    {
        // Open the numbers file
        std::string filepath = m_CurrentDirPath + filename;
        GSH_INFO("Opening %s", filepath.c_str());
        FILE *f = fopen(filepath.c_str(), flag);
        GSH_INFO("%s\n", (!f ? "Fail :(" : "OK"));
        if (!f) {
            // Create the numbers file if it doesn't exist
            GSH_INFO("No file found, creating a new file... ");
            f = fopen(filepath.c_str(), "w+");
            GSH_INFO("%s\n", (!f ? "Fail :(" : "OK"));
            if (!f) {
                GSH_ERROR("%s (%d)\n", strerror(errno), -errno);
            }
        }
        return f;
    }

    bool FileSystem::closeFile(FILE* f)
    {
        GSH_TRACE("Closing file... ");
        int err = fclose(f);
        GSH_INFO("%s\n", (err < 0 ? "Fail :(" : "OK"));
        if (err < 0) 
        {
            GSH_ERROR("%s (%d)\n", strerror(errno), -errno);
            return false;
        }
        return true;
    }

    void FileSystem::printFile(FILE* f)
    {
        GSH_ERROR("file:");
        fseek(f, 0, SEEK_SET);
        int c = fgetc(f);
        printf("%c", c);
        while (c != EOF) {
            c = fgetc(f);
            printf("%c", c);
        }

    }

    DIR* FileSystem::openDir(const std::string& _dirPath)
    {
        std::string dirPath = m_CurrentDirPath + _dirPath;
        GSH_INFO("Opening the %s directory... ", dirPath.c_str());
        DIR *d = opendir(dirPath.c_str());
        GSH_INFO("%s\n", (!d ? "Fail :(" : "OK"));
        if (!d) {
            GSH_ERROR("%s (%d)\n", strerror(errno), -errno);
        }
        return d;
    }

    bool FileSystem::closeDir(DIR* dir)
    {
        GSH_TRACE("Closing directory... ");
        int err = closedir(dir);
        GSH_INFO("%s\n", (err < 0 ? "Fail :(" : "OK"));
        if (err < 0) 
        {
            GSH_ERROR("%s (%d)\n", strerror(errno), -errno);
            return false;
        }
        return true;
    }

    const string& FileSystem::changeDir(const std::string& dirPath)
    {
        m_CurrentDirPath += dirPath;
        return m_CurrentDirPath;
    }

    void FileSystem::printDir(DIR* dir)
    {
        while (true) 
        {
            struct dirent *e = readdir(dir);
            if (!e) { break; }
            GSH_INFO("    %s\n", e->d_name);
        }
    }
}