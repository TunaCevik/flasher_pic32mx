import os
import subprocess
import sys
from datetime import date

CHANGELOG_PATH = "CHANGELOG.md"

# Category order also doubles as changelog section order (most significant first).
CATEGORY_PREFIXES = {
    "Features": ["feat:", "new-project"],
    "Fixes": ["fix:"],
    "Debugging Notes": ["debug:"],
    "Errors Found": ["error:"],
    "Refactors": ["refactor:"],
    "Documentation": ["docs:"],
}
CATEGORY_ORDER = [
    "Breaking Changes",
    "Features",
    "Fixes",
    "Debugging Notes",
    "Errors Found",
    "Refactors",
    "Documentation",
    "Other",
]


def run_cmd(command):
    result = subprocess.run(command, shell=True, text=True, capture_output=True)
    return result.stdout.strip()


def get_latest_tag():
    tag = run_cmd("git describe --tags --abbrev=0")
    if not tag:
        return "v0.0.0"
    return tag


def classify_commit(msg):
    # Breaking-change check runs first and unconditionally, so it can't be
    # masked by a message that also starts with a patch/minor prefix (e.g.
    # "fix: swap PGC/PGD pins -- BREAKING CHANGE: wire mapping changed" must
    # still count as breaking, not just a fix).
    if "BREAKING CHANGE" in msg or "!" in msg.split(":")[0]:
        return "Breaking Changes"
    for category, prefixes in CATEGORY_PREFIXES.items():
        if any(msg.startswith(p) for p in prefixes):
            return category
    return "Other"


def display_text(msg, category):
    for prefix in CATEGORY_PREFIXES.get(category, []):
        if msg.startswith(prefix):
            return msg[len(prefix):].strip()
    return msg


def bump_version():
    latest_tag = get_latest_tag()
    print(f"Current Latest Version: {latest_tag}")

    version_clean = latest_tag.lstrip("v")
    parts = version_clean.split(".")
    major, minor, patch = int(parts[0]), int(parts[1]), int(parts[2])

    # If no tags exist yet, read all commits from the repository history --
    # this is also what makes the very first run pick up the entire
    # debugging trail (debug:/error:/fix: commits included) for the
    # changelog's first entry, not just whatever's since some prior tag.
    if latest_tag == "v0.0.0":
        commit_msgs = run_cmd("git log --pretty=format:%s").split("\n")
    else:
        commit_msgs = run_cmd(f"git log {latest_tag}..HEAD --pretty=format:%s").split("\n")

    bump_type = "patch"  # Default is patch
    for msg in commit_msgs:
        category = classify_commit(msg)
        if category == "Breaking Changes":
            bump_type = "major"
            break
        elif category == "Features":
            bump_type = "minor"
        # every other category (Fixes/Debugging Notes/Errors Found/
        # Refactors/Documentation/Other) stays at whatever bump_type is
        # already set -- never downgrades a minor/major back to patch.

    if bump_type == "major":
        major += 1
        minor = 0
        patch = 0
    elif bump_type == "minor":
        minor += 1
        patch = 0
    else:
        patch += 1

    new_tag = f"v{major}.{minor}.{patch}"
    print(f"New Version Determined: {new_tag} ({bump_type} bump)")
    return new_tag, commit_msgs, bump_type


def write_changelog(new_tag, commit_msgs, path=CHANGELOG_PATH):
    """Prepend a new dated, categorized section to CHANGELOG.md -- a
    separate, auto-generated record of every debug/fix/feature commit this
    release covers, distinct from the hand-written narrative entries in
    docs/Devlog/."""
    buckets = {category: [] for category in CATEGORY_ORDER}
    for msg in commit_msgs:
        if not msg:
            continue
        category = classify_commit(msg)
        buckets[category].append(display_text(msg, category))

    section_lines = [f"## {new_tag} — {date.today().isoformat()}", ""]
    for category in CATEGORY_ORDER:
        entries = buckets[category]
        if not entries:
            continue
        section_lines.append(f"### {category}")
        for entry in entries:
            section_lines.append(f"- {entry}")
        section_lines.append("")
    new_section = "\n".join(section_lines).rstrip() + "\n"

    header = "# Changelog\n\n"
    body = ""
    if os.path.exists(path):
        with open(path, "r") as f:
            existing = f.read()
        body = existing[len(header):] if existing.startswith(header) else existing

    with open(path, "w") as f:
        f.write(header + new_section + "\n" + body)

    return path


if __name__ == "__main__":
    new_version, commit_msgs, bump_type = bump_version()

    confirm = input(f"Do you want to create and push tag '{new_version}'? (y/n): ")
    if confirm.lower() == "y":
        changelog_path = write_changelog(new_version, commit_msgs)
        print(f"Wrote {changelog_path} entry for {new_version}")

        print(run_cmd(f"git add {changelog_path}"))
        print(run_cmd(f'git commit -m "docs: changelog for {new_version}"'))

        current_branch = run_cmd("git rev-parse --abbrev-ref HEAD")
        print(run_cmd(f"git tag {new_version}"))
        print(run_cmd(f"git push origin {current_branch}"))
        print(run_cmd(f"git push origin {new_version}"))
        print("Success! Changelog committed, new tag pushed to remote repository.")
    else:
        print("Cancelled.")
