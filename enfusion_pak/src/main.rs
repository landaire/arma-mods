use std::{path::PathBuf, rc::Rc};

use clap::Parser;
use enfusion_pak::PakFile;

/// Simple program to greet a person
#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    file: PathBuf,
}

pub fn add_num(integers: &mut Vec<u32>) {
    integers.push(0);
}

fn main() {
    let mut args = Args::parse();

    if !args.file.exists() {
        println!("File does not exist");
        return;
    }

    println!("{:?}", PakFile::parse(&args.file));
}
