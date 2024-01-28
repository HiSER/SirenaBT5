<?php

require_once 'hash.php';
require_once 'hex.php';

class WRITE
{
	public $command = [];

	public function __construct(string $file, int $align)
	{
		$hex = new HEX($file);

		$length = count($hex->data);
		if ($length == 0) return;

		$length = $hex->align($align);

		for ($i = 0; $i < $length; $i += $align)
		{
			$cmd = '<' . sprintf('%04X', $i);
			for ($n = $i; $n < ($i + $align); $n++)
			{
				$cmd .= sprintf('%02X', $hex->data[$n]);
			}
			$cmd .= sprintf('%04X', HASH::crc16arr($hex->data, $i, $align));
			$this->command[] = $cmd;
		}
	}
}