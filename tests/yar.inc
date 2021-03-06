<?php
define ("YAR_SERVER_HOSTNAME", "localhost");
define ("YAR_SERVER_PORT", 8964);
define ("YAR_SERVER_ADDRESS", YAR_SERVER_HOSTNAME.":".YAR_SERVER_PORT);
define ("YAR_SERVER_HTTP_ADDRESS", "http://" . YAR_SERVER_HOSTNAME.":".YAR_SERVER_PORT . "/");

function yar_server_start($code = '"Hello world";', $no_router = FALSE, $expose = TRUE) {
	$php_executable = getenv('TEST_PHP_EXECUTABLE');
	$doc_root = __DIR__;
	$router = "index.php";

	if ($code) {
		file_put_contents($doc_root . '/' . $router, '<?php ' . $code . ' ?>');
	}

	$descriptorspec = array(
		0 => STDIN,
		1 => STDOUT,
		2 => STDERR,
	);

	if (substr(PHP_OS, 0, 3) == 'WIN') {
        $cmd = "{$php_executable} -d yar.expose_info=" . ($expose? 1 : 0) . " -t {$doc_root} -S " . YAR_SERVER_ADDRESS;
		if (!$no_router) {
			$cmd .= " {$router}";
		}

		$handle = proc_open(addslashes($cmd), $descriptorspec, $pipes, $doc_root, NULL, array("bypass_shell" => true,  "suppress_errors" => true));
	} else {
        $cmd = "exec {$php_executable} -d yar.expose_info=" . ($expose? 1 : 0) . " -t {$doc_root} -S " . YAR_SERVER_ADDRESS;
		if (!$no_router) {
			$cmd .= " {$router}";
        }
        $cmd .= " 2>/dev/null";

		$handle = proc_open($cmd, $descriptorspec, $pipes, $doc_root);
	}
	
	// note: even when server prints 'Listening on localhost:8964...Press Ctrl-C to quit.'
	//       it might not be listening yet...need to wait until fsockopen() call returns
    $i = 0;
    while (($i++ < 5) && !($fp = @fsockopen(YAR_SERVER_HOSTNAME, YAR_SERVER_PORT))) {
        usleep(10000);
    }

    if ($fp) {
        fclose($fp);
    }

	register_shutdown_function(
		function($handle) use($router) {
			proc_terminate($handle);
			@unlink(__DIR__ . "/{$router}");
		},
			$handle
		);
	// don't bother sleeping, server is already up
	// server can take a variable amount of time to be up, so just sleeping a guessed amount of time
	// does not work. this is why tests sometimes pass and sometimes fail. to get a reliable pass
	// sleeping doesn't work.
}
?>

