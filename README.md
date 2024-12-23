# Cpp Debugger

## Build Instructions

1. **Install vcpkg**  
   Make sure you have vcpkg installed on your system.

2. **Create CMakeUserPresets.json in the root directory**  
   Configure the presets and set the VCPKG_ROOT environment variable. Use the following template:  
```json
   {
     "version": 1,
     "configurePresets": [
       {
         "name": "<user_preset_name>",
         "inherits": "vcpkg",
         "environment": {
           "VCPKG_ROOT": "<Path_to_vcpkg>"
         }
       }
     ]
   }
```

3. **Build the Project**  
   Run the following commands:
```
   mkdir build && cd build  
   cmake .. --preset <user_preset_name>  
   cmake --build .
```
## Run the Debugger

Execute the following command to run the debugger:  
`./tools/cdb`

## Run Tests

To execute the test suite, use:  
`./test/tests`
