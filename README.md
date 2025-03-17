# project-manager

### Usage
pm <args...>

pm -l lists all projects.

pm -l --lang lists all languages.

pm -l -u <user> lists all languages.



pm add <location> manualy adds a project to the index.

pm create <lang> <name> <?-o> creates a project in the correct folder, -o to override if the project already exists.

pm delete <name | project id> deletes the given project.

pm makelang <lang> creates a new option for projects to be stored under this language.

pm removelang <lang> removes the option for projects to be stored under this language.

pm refresh <?lang> refresh all languages or only the specified one.