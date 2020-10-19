project structure:

PROJECT_ROOT = simavr_${PROJECT}

${PROJECT_ROOT}/examples/
${PROJECT_ROOT}/examples/board_*/*
${PROJECT_ROOT}/examples/parts/*
--> ${PROJECT_ROOT}/examples/parts/dtime/*
--> ${PROJECT_ROOT}/examples/parts/spi_flash/*
--> ${PROJECT_ROOT}/examples/parts/spi_sram/*

ln -s ${SIMAVR_ROOT}/simavr ${PROJECT_ROOT}/simavr
ln -s ${SIMAVR_ROOT}/Makefile ${PROJECT_ROOT}/Makefile
ln -s ${SIMAVR_ROOT}/Makefile.common ${PROJECT_ROOT}/Makefile.common
