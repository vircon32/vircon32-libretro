cmake_minimum_required(VERSION
	3.17.5 # foreach(loop-var... IN ZIP_LISTS <lists>...)
)

include_guard()

function(generate_code_to_embed_binary asset_name asset_path byte_type constexpr null_terminate out_generated_header out_generated_implementation)
	if (NOT EXISTS "${asset_path}")	
		message(FATAL_ERROR "The asset '${asset_name}' does not exist in \"${asset_path}\"")
	endif()

	file(READ "${asset_path}" file_contents HEX)
	string(LENGTH "${file_contents}" file_contents_length)

	string(MAKE_C_IDENTIFIER "${asset_name}" asset_name_identifier)

	math(EXPR file_bytes "${file_contents_length} / 2")
	math(EXPR file_contents_modulo_2 "${file_contents_length} % 2")

	if (NOT file_contents_modulo_2 EQUAL 0)
		message(FATAL_ERROR "File length in hexadecimal must be a multiple of 2")
	endif()

	if (NOT DEFINED null_terminate)
		message(FATAL_ERROR "'null_terminate' must be provided")
	endif()

	if (null_terminate)
		math(EXPR file_bytes "${file_bytes} + 1")
	endif()

	if (NOT DEFINED constexpr)
		message(FATAL_ERROR "'constexpr' must be provided")
	endif()
	
	if (NOT byte_type)
		message(FATAL_ERROR "'byte_type' must be provided and must not be empty: \"${byte_type}\"")
	endif()

	set(bytes_per_line 64)
    string(REPEAT "[0-9a-f]" ${bytes_per_line} column_pattern)
    string(REGEX REPLACE "(${column_pattern})" "\\1\n" code "${file_contents}")

    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," code "${code}")

	if (null_terminate)
		string(APPEND code "0x0") # NULL-terminator
	endif()

	set(partial_declaration "${byte_type} embedded_${asset_name_identifier}[${file_bytes}]")
	set(code "${partial_declaration} = {\n${code}\n};\n")

	set(header "#pragma once\n\n")
	set(implementation "")

	if (constexpr)
		string(APPEND header "#ifndef __cplusplus\n#error \"'constexpr' is a C++ feature\"\n#endif\n\nconstexpr ${code}")
	else()
		string(APPEND header "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\nextern const ${partial_declaration};\n\n#ifdef __cplusplus\n}\n#endif\n")
		string(APPEND implementation "#include \"${asset_name_identifier}.h\"\n\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n\nconst ${code}\n#ifdef __cplusplus\n}\n#endif\n")
	endif()

	set(${out_generated_header} "${header}" PARENT_SCOPE)
	set(${out_generated_implementation} "${implementation}" PARENT_SCOPE)
endfunction()

# The variable is set when including this file
set(_path_to_embed_binary_myself ${CMAKE_CURRENT_LIST_FILE})

# embed_binaries(<generated_target_name>
#	[ASSET NAME <name> PATH <path> [BYTE_TYPE <c-cpp-type>] [CONSTEXPR] [NULL_TERMINATE]]...)
function(embed_binaries target_name)
	if (NOT DEFINED target_name)	
		message(FATAL_ERROR "Missing required argument 'TARGET'")
	endif()

	# Fill these lists with the arguments for each ASSET
	# so that we can loop while zipping over them
	set(asset_NAMEs)
	set(asset_PATHs)
	set(asset_CONSTEXPRs)
	set(asset_BYTE_TYPEs)
	set(asset_NULL_TERMINATEs)

	set(asset_args_to_parse "${ARGN}")
	list(FIND asset_args_to_parse ASSET first_asset_index)

	if (first_asset_index EQUAL -1)
		message(WARNING "No assets to embed in target '${target_name}'")

		# Generate empty library
		add_library("${target_name}" INTERFACE)
		return()
	elseif(NOT first_asset_index EQUAL 0)
		message(FATAL_ERROR "embed_binaries: expected 'ASSET' after <target_name> (\"${target_name}\")")
	endif()
	
	while(asset_args_to_parse) # While not empty
		list(POP_FRONT asset_args_to_parse must_be_ASSET) # Remove "ASSET" from the front
		if (NOT must_be_ASSET STREQUAL "ASSET")
			message(FATAL_ERROR "Bug detected: \"ASSET\" expected, got \"${must_be_ASSET}\". Remaining args: ${asset_args_to_parse}")
		endif()

		list(FIND asset_args_to_parse ASSET asset_arg_count)

		# Current sublist. If ASSET is not found, FIND returns -1,
		# which SUBLIST interprets as taking the list until the end
		list(SUBLIST asset_args_to_parse 0 ${asset_arg_count} current_asset_args)

		# Remaining sublist
		if (asset_arg_count EQUAL -1)
			set(asset_args_to_parse)
		else()
			list(SUBLIST asset_args_to_parse ${asset_arg_count} -1 asset_args_to_parse)
		endif()

		set(asset_options CONSTEXPR NULL_TERMINATE)
		set(asset_required_args NAME PATH)
		set(asset_optional_args MODE BYTE_TYPE)
		set(asset_optional_args_defaults "extern" "unsigned char")
		set(asset_args ${asset_required_args} ${asset_optional_args})
		set(asset_list_args)

		cmake_parse_arguments(asset
			"${asset_options}" "${asset_args}" "${asset_list_args}"
			"${current_asset_args}"
		)

		if (asset_UNPARSED_ARGUMENTS)
			foreach(unrecognized IN LISTS asset_UNPARSED_ARGUMENTS)
				message(SEND_ERROR "Unrecognized argument: '${unrecognized}'")
			endforeach()
			message(FATAL_ERROR "embed_binaries: unrecognized argument(s) (see above) for ASSET with args: \"${current_asset_args}\"")
		endif()

		foreach(param_name IN LISTS asset_options)
			list(APPEND asset_${param_name}s ${asset_${param_name}}) # Always defined to either true or false
		endforeach()

		foreach(param_name IN LISTS asset_required_args)
			if (NOT DEFINED asset_${param_name})
				message(FATAL_ERROR "ASSET '${param_name}' missing in ASSET with args: \"${current_asset_args}\"")
			endif()

			list(APPEND asset_${param_name}s ${asset_${param_name}})
		endforeach()

		foreach(param_name default_value IN ZIP_LISTS asset_optional_args asset_optional_args_defaults)
			if (NOT DEFINED asset_${param_name})
				set(asset_${param_name} ${default_value})
			endif()

			list(APPEND asset_${param_name}s ${asset_${param_name}})
		endforeach()
	endwhile()

	set(library_type OBJECT)
	set(header_visibility PRIVATE)

	list(FIND asset_CONSTEXPRs FALSE non_constexpr_index)
	if (non_constexpr_index EQUAL -1)
		set(library_type INTERFACE)
		set(header_visibility INTERFACE)
	else()
		enable_language(C)
	endif()

	add_library("${target_name}" "${library_type}")
	#set_target_properties("${target_name}" PROPERTIES LINKER_LANGUAGE C) # CMake seemed to not be able to detect it (?)
	target_include_directories("${target_name}" INTERFACE "${CMAKE_CURRENT_BINARY_DIR}")

	if (non_constexpr_index EQUAL -1)
		target_compile_features("${target_name}" INTERFACE cxx_constexpr)
	endif()

	foreach(
		asset_NAME asset_PATH asset_CONSTEXPR asset_BYTE_TYPE asset_NULL_TERMINATE
		IN ZIP_LISTS
		asset_NAMEs asset_PATHs asset_CONSTEXPRs asset_BYTE_TYPEs asset_NULL_TERMINATEs)
		string(MAKE_C_IDENTIFIER "${asset_NAME}" asset_name_identifier)

		get_filename_component(asset_PATH ${asset_PATH} ABSOLUTE)

		if (NOT EXISTS "${asset_PATH}")
			message(FATAL_ERROR "The asset '${asset_NAME}' does not exist in \"${asset_PATH}\"")
		endif()

		set(generated_code_directory "${CMAKE_CURRENT_BINARY_DIR}/embedded")
		set(generated_header_path "${generated_code_directory}/${asset_name_identifier}.h")
		set(generated_implementation_path "${generated_code_directory}/${asset_name_identifier}.c")

		target_sources("${target_name}" "${header_visibility}" "${generated_header_path}")
		if (asset_CONSTEXPR)
			set(generated_asset_files "${generated_header_path}")
		else()
			set(generated_asset_files "${generated_header_path}" "${generated_implementation_path}")
			target_sources("${target_name}" PRIVATE "${generated_implementation_path}")
		endif()

		add_custom_command(OUTPUT ${generated_asset_files}
			COMMAND "${CMAKE_COMMAND}" -E make_directory "${generated_code_directory}"
			COMMAND "${CMAKE_COMMAND}"
				ARGS
					"-Dasset_name=${asset_NAME}"
					"-Dasset_path=${asset_PATH}"
					"-Dconstexpr=${asset_CONSTEXPR}"
					"-Dbyte_type=${asset_BYTE_TYPE}"
					"-Dnull_terminate=${asset_NULL_TERMINATE}"
					"-Dgenerated_header_path=${generated_header_path}"
					"-Dgenerated_implementation_path=${generated_implementation_path}"
					-P "${_path_to_embed_binary_myself}"
			DEPENDS "${asset_PATH}"
			COMMENT "Embedding binary asset '${asset_NAME}' from \"${asset_PATH}\"..."
			VERBATIM
		)
	endforeach()
endfunction()

function(write_embedded_binary_code asset_name asset_path byte_type constexpr null_terminate generated_header_path generated_implementation_path)
	generate_code_to_embed_binary("${asset_name}" "${asset_path}" "${byte_type}" "${constexpr}" "${null_terminate}" generated_header generated_implementation)

	file(WRITE "${generated_header_path}" "${generated_header}")

	if (generated_implementation STREQUAL "")
		if (EXISTS "${generated_implementation_path}")
			file(REMOVE "${generated_implementation_path}")
		endif()
	else()
		file(WRITE "${generated_implementation_path}" "${generated_implementation}")
	endif()
endfunction()

# Running in script mode
# https://stackoverflow.com/questions/51427538/cmake-test-if-i-am-in-scripting-mode
if(CMAKE_SCRIPT_MODE_FILE AND NOT CMAKE_PARENT_LIST_FILE)
    foreach(variable "asset_name" "asset_path" "byte_type" "constexpr" "null_terminate" "generated_header_path" "generated_implementation_path")
        if (NOT DEFINED ${variable})
            message(FATAL_ERROR "'${variable}' is not defined")
        endif()
    endforeach()

    write_embedded_binary_code("${asset_name}" "${asset_path}" "${byte_type}" "${constexpr}" "${null_terminate}" "${generated_header_path}" "${generated_implementation_path}")
endif()
