# PTOOP
This is an implementation of OOP in C using hash-tables.

## Objects

Each object is a variable of type `prototype *`.

```C
prototype *car;
```

To allocate memory for an object, you can use `palloc`:
```C
prototype *palloc(int len);
```
`len` specifies the length of the array within the hash-table.
A good value might be the number of data fields and methods your object will contain:
```C
prototype *car = palloc(2);
```
Do not set it to a value less than `1`.

## Adding data fields and methods to an object
Constructors are simply functions that take in an object and add fields and methods to it:
```C
prototype *Car(prototype *pt)
{
    return pt;
}
```

### Data fields
To add a data field to an object, use the `field` macro:
```C
prototype *Car(prototype *pt, char *name, int year, char *color)
{
    field(pt, char *, name, free) = strdup(name);
    field(pt, int, year) = year;
    field(pt, char *, color, free) = strdup(color);

    return pt;
}

prototype *File(prototype *pt, char *path)
{
    field(pt, FILE *, fp, fclose) = fopen(path, "rw");
    return pt;
}
```
The first argument of `field` is the object to which the field is added.

The second argument is the type of the field.

The third argument is the identifier used to refer to that field.

The fourth argument is optional and let's you specify a clean-up function that will be called on that field when its parent object is freed. Do not specify a clean-up function(Not even `NULL`) for non-pointer types such as `int`, `double`, etc. It's better that you use `NULL` as the clean-up function for pointer types that you don't want to be freed instead of leaving it out to prevent unnecessary boxing.

Simply assign the result of a macro a value to initialize it.

#### Working with data fields
To work with the fields within an object, you can use the `access` macro:
```C
void Car_print(prototype *pt)
{
    printf("%s %d %s\n", access(pt, char *, name), access(pt, int, year), access(pt, char *, color));
    access(pt, int, year) += 1;
}
```
Do not use `field` to work with data fields that already exist within the object. Doing so will call the clean-up function on the already existing field and replace it. This might be useful if you need to change the type of an already existing field within an object or override it.

### Methods
To add an overridable method to an object, you can use the `method` macro:
```C
prototype *Car(prototype *pt, char *name, int year, char *color)
{
    field(pt, char *, name, free) = strdup(name);
    field(pt, int, year) = year;
    field(pt, char *, color, free) = strdup(color);
    method(pt, void, print) = Car_print;

    return pt;
}
```
Do not use `method` on an object without assigning it a function pointer. Doing so will set the value of the function pointer associated with that identifier within the object to `NULL`.

If your method doesn't need overriding, you can refrain from adding it to the object itself and simply use it wherever the method is declared.

Note:
- Your methods first argument must be the object it gets called on.
- Overridable methods should not contain arguments of integer types with sizes less than `int`(e.g `char`, `_Bool`, `short`) or floating-point types with sizes less than `double` (e.g `float`). This is due to C's default promotion rules in absence of a function's declaration. All other types (e.g `long long`, `char *`, etc) are fine to use.
- A method and a data field within an object cannot share the same identifier, if you use `field` and `method` to try such a thing one of them will override the other.

#### Calling methods

To call a normal method on an object use the `call` macro:
```C
void Car_setYear(prototype *pt, int year)
{
    access(pt, int, year) = year;
}
```
```C
call(pt, Car_setYear, 2002);
```
After the identifier of the method, its arguments other than the first (`prototype *`) should be given.

To call an overridable method on an object use the `apply` macro:
```C
apply(pt, void, print); /* Calls the function stored as 'print', 'Car_print' on pt */
```
Similar to `call` after the identifier of the method, its arguments other than the first (`prototype *`) should be given.

Note:
- When calling an overridable method, any necessary casting must be done explicitly by you:

```C
void Car_giveYearx(prototype *pt, double x)
{
    return access(pt, int, year) * x;
}
```
```C
prototype *car = Car(palloc(3), "BMW", 2000, "Black");
method(car, void, year_x) = Car_giveYearx;

apply(car, void, year_x, 10); /* This won't work correctly */

apply(car, void, year_x, 10.0); /* But this is fine */
```

## Memory management

When you don't need an object anymore, you can use `pfree` to deallocate it:
```C
pfree(car);
```
Because `pfree` will call the clean-up functions you've specified for each field in the object, you don't need to manually free them(unless you've intentionally specified the clean-up function as `NULL`):
```C
prototype *A = palloc(1);
prototype *B = palloc(1);
prototype *C = palloc(1);

field(A, prototype *, next, pfree) = B;
field(B, void *, mem, free) = malloc(8);
field(B, prototype *, next, pfree) = C;
field(C, prototype *, next, pfree) = A;

pfree(A); /* This will free A, B and C and their fields. Notice the fact that C contains a pointer to A doesn't cause a problem */
```
Note:
- if you change the value of field you've specified a clean-up function for, it might cause problems:
```C
prototype *pt = palloc(2);
field(pt, int *, array, free) = malloc(sizeof(int) * 10);
acess(pt, int *, array) += 5; /* This is not good */
pfree(pt);
```

## Inheritance

To achieve inheritance you can call multiple constructor functions on the same object:
```C
prototype *Veichle(prototype *pt, char *class)
{
    field(pt, char *, class, NULL) = class;
    return pt;
}

prototype *Car(prototype *pt, char *name, int year, char *color)
{
    Veichle(pt, "Car");
    field(pt, char *, name, free) = strdup(name);
    field(pt, int, year) = year;
    field(pt, char *, color, free) = strdup(color);
    method(pt, void, print) = Car_print;

    return pt;
}
```
This way an object can inherit from multiple classes:
```C
prototype *Parser(prototype *pt, char *path)
{
    field(pt, FILE *, fp, fclose) = fopen(path, "r");
    return pt;
}

prototype *List(prototype *pt)
{
    field(pt, prototype *, head, pfree) = NULL;
    field(pt, prototype *, tail, pfree) = NULL;
    return pt;
}
```
```C
prototype *weird_object = Parser(List(Car(palloc(5))));
```

Note:
- If the classes an object inherits from have conflicting identifiers, the conflicting fields(or methods) of the later classes will call the clean-up functions(if any) on the fields(or methods) of the previous classes before overriding them.

## Using strings as identifiers
Simply adding the prefix `pt` to the name of the any macros above(except for `call`) allows you to work with objects through string identifiers:
```C
prototype *pt = palloc(1000);
for (int i = 0; i < 1000; i++) {
    char a[7];
    sprintf(a, "car%d", i);
    ptfield(pt, prototype *, a, pfree) = Car(palloc(3));
}
for (int i = 0; i < 1000; i++) {
    char a[7];
    sprintf(a, "car%d", i);
    ptapply(ptaccess(pt, prototype *, a), void, "print");
}
pfree(pt);
```
