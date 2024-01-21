CREATE TABLE schema_version (
    version INTEGER NOT NULL PRIMARY KEY
);

CREATE TABLE wad (
    wad_id        INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    path          TEXT    NOT NULL,
    last_modified TEXT    NOT NULL,
    checksum      TEXT    NOT NULL
);

CREATE UNIQUE INDEX wad_path ON wad(path);
CREATE UNIQUE INDEX wad_checksum ON wad(checksum);

CREATE TABLE texture (
    texture_id    INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    wad_id        INTEGER NOT NULL,
    offset        INTEGER NOT NULL,
    name          TEXT    NOT NULL,
    width         INTEGER NOT NULL,
    height        INTEGER NOT NULL,
    category      INTEGER NOT NULL,
    checksum      TEXT    NOT NULL,

    FOREIGN KEY(wad_id) REFERENCES wad(wad_id) ON DELETE CASCADE
);

CREATE INDEX texture_category ON texture(category);
CREATE UNIQUE INDEX texture_checksum ON texture(checksum);

INSERT INTO schema_version (version) VALUES (1);