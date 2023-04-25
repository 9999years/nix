#pragma once
///@file

#include "types.hh"
#include "hash.hh"
#include "canon-path.hh"
#include "attrs.hh"
#include "url.hh"

#include <memory>

namespace nix { class Store; class StorePath; struct InputAccessor; }

namespace nix::fetchers {

struct InputScheme;

/**
 * The Input object is generated by a specific fetcher, based on the
 * user-supplied input attribute in the flake.nix file, and contains
 * the information that the specific fetcher needs to perform the
 * actual fetch.  The Input object is most commonly created via the
 * "fromURL()" or "fromAttrs()" static functions which are provided
 * the url or attrset specified in the flake file.
 */
struct Input
{
    friend struct InputScheme;

    std::shared_ptr<InputScheme> scheme; // note: can be null
    Attrs attrs;

public:
    static Input fromURL(const std::string & url);

    static Input fromURL(const ParsedURL & url);

    static Input fromAttrs(Attrs && attrs);

    ParsedURL toURL() const;

    std::string toURLString(const std::map<std::string, std::string> & extraQuery = {}) const;

    std::string to_string() const;

    Attrs toAttrs() const;

    /**
     * Check whether this is a "direct" input, that is, not
     * one that goes through a registry.
     */
    bool isDirect() const;

    /**
     * Check whether this is a "locked" input, that is,
     * one that contains a commit hash or content hash.
     */
    bool isLocked() const;

    /**
     * Only for relative path flakes, i.e. 'path:./foo', returns the
     * relative path, i.e. './foo'.
     */
    std::optional<std::string> isRelative() const;

    bool operator ==(const Input & other) const;

    bool contains(const Input & other) const;

    /**
     * Fetch the entire input into the Nix store, returning the
     * location in the Nix store and the locked input.
     */
    std::pair<StorePath, Input> fetchToStore(ref<Store> store) const;

    /**
     * Return an InputAccessor that allows access to files in the
     * input without copying it to the store. Also return a possibly
     * unlocked input.
     */
    std::pair<ref<InputAccessor>, Input> getAccessor(ref<Store> store) const;

    Input applyOverrides(
        std::optional<std::string> ref,
        std::optional<Hash> rev) const;

    void clone(const Path & destDir) const;

    void putFile(
        const CanonPath & path,
        std::string_view contents,
        std::optional<std::string> commitMsg) const;

    std::string getName() const;

    // Convenience functions for common attributes.
    std::string getType() const;
    std::optional<Hash> getNarHash() const;
    std::optional<std::string> getRef() const;
    std::optional<Hash> getRev() const;
    std::optional<uint64_t> getRevCount() const;
    std::optional<time_t> getLastModified() const;

    // For locked inputs, returns a string that uniquely specifies the
    // content of the input (typically a commit hash or content hash).
    std::optional<std::string> getFingerprint(ref<Store> store) const;
};

/**
 * The InputScheme represents a type of fetcher.  Each fetcher
 * registers with nix at startup time.  When processing an input for a
 * flake, each scheme is given an opportunity to "recognize" that
 * input from the url or attributes in the flake file's specification
 * and return an Input object to represent the input if it is
 * recognized.  The Input object contains the information the fetcher
 * needs to actually perform the "fetch()" when called.
 */
struct InputScheme
{
    virtual ~InputScheme()
    { }

    virtual std::optional<Input> inputFromURL(const ParsedURL & url) const = 0;

    virtual std::optional<Input> inputFromAttrs(const Attrs & attrs) const = 0;

    virtual ParsedURL toURL(const Input & input) const;

    virtual Input applyOverrides(
        const Input & input,
        std::optional<std::string> ref,
        std::optional<Hash> rev) const;

    virtual void clone(const Input & input, const Path & destDir) const;

    virtual void putFile(
        const Input & input,
        const CanonPath & path,
        std::string_view contents,
        std::optional<std::string> commitMsg) const;

    virtual std::pair<ref<InputAccessor>, Input> getAccessor(ref<Store> store, const Input & input) const = 0;

    virtual bool isDirect(const Input & input) const
    { return true; }

    virtual bool isLocked(const Input & input) const
    { return false; }

    virtual std::optional<std::string> isRelative(const Input & input) const
    { return std::nullopt; }

    virtual std::optional<std::string> getFingerprint(ref<Store> store, const Input & input) const;

    virtual void checkLocks(const Input & specified, const Input & final) const;
};

void registerInputScheme(std::shared_ptr<InputScheme> && fetcher);

}
