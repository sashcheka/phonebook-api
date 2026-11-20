CREATE TABLE contacts (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    phone TEXT NOT NULL,
    address TEXT NOT NULL
);

WITH sample_contacts(id, name) AS (
    VALUES (1, 'Ada Bennett'), (2, 'Ada Brooks'), (3, 'Ada Hayes'),
           (4, 'Ada Miller'), (5, 'Ada Reed'), (6, 'Amir Bennett'),
           (7, 'Amir Brooks'), (8, 'Amir Hayes'), (9, 'Amir Miller'),
           (10, 'Amir Reed'), (11, 'Elena Bennett'), (12, 'Elena Brooks'),
           (13, 'Elena Hayes'), (14, 'Elena Miller'), (15, 'Elena Reed'),
           (16, 'Grace Bennett'), (17, 'Grace Brooks'), (18, 'Grace Hayes'),
           (19, 'Grace Miller'), (20, 'Grace Reed'), (21, 'Jun Bennett'),
           (22, 'Jun Brooks'), (23, 'Jun Hayes'), (24, 'Jun Miller'),
           (25, 'Jun Reed'), (26, 'Leah Bennett'), (27, 'Leah Brooks'),
           (28, 'Leah Hayes'), (29, 'Leah Miller'), (30, 'Leah Reed'),
           (31, 'Milo Bennett'), (32, 'Milo Brooks'), (33, 'Milo Hayes'),
           (34, 'Milo Miller'), (35, 'Milo Reed'), (36, 'Nora Bennett'),
           (37, 'Nora Brooks'), (38, 'Nora Hayes'), (39, 'Nora Miller'),
           (40, 'Nora Reed'), (41, 'Aria Carter'), (42, 'Aria Davis'),
           (43, 'Aria Morgan'), (44, 'Aria Parker'), (45, 'Aria Sutton'),
           (46, 'Felix Carter'), (47, 'Felix Davis'), (48, 'Felix Morgan'),
           (49, 'Felix Parker'), (50, 'Felix Sutton'), (51, 'Hana Carter'),
           (52, 'Hana Davis'), (53, 'Hana Morgan'), (54, 'Hana Parker'),
           (55, 'Hana Sutton'), (56, 'Leo Carter'), (57, 'Leo Davis'),
           (58, 'Leo Morgan'), (59, 'Leo Parker'), (60, 'Leo Sutton'),
           (61, 'Maya Carter'), (62, 'Maya Davis'), (63, 'Maya Morgan'),
           (64, 'Maya Parker'), (65, 'Maya Sutton'), (66, 'Owen Carter'),
           (67, 'Owen Davis'), (68, 'Owen Morgan'), (69, 'Owen Parker')
)
INSERT INTO contacts (id, name, phone, address)
SELECT id, name,
       printf('+1-202-555-%04d', 99 + id),
       printf('%d Example Avenue', 100 + id)
FROM sample_contacts
ORDER BY id;
